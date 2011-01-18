#include "Fuji.h"
#include "MFFileSystem.h"
#include "MFHeap.h"
#include "MFSound.h"

#include "GHEd.h"

#if !defined(MAKE_LEGAL)

#include "VAGFile.h"

#include <stdio.h>
#include <string.h>

#define XA_BLOCK_SIZE 28
#define XA_BLOCK_BYTES (2+(XA_BLOCK_SIZE/2))

float f[5][2] =
{
	{ 0.0, 0.0 },
	{  60.0 / 64.0,  0.0 },
	{  115.0 / 64.0, -52.0 / 64.0 },
	{  98.0 / 64.0, -55.0 / 64.0 },
	{  122.0 / 64.0, -60.0 / 64.0 }
};

float samples[XA_BLOCK_SIZE];

int UnpackVAGBlock(int16 *pOutputBuffer, const char *pVAGBuffer)
{
	int predict_nr, shift_factor, flags;
	int i;
	int d, s;
	static float s_1 = 0.0f;
	static float s_2 = 0.0f;

	predict_nr = *(uint8*)pVAGBuffer++;
	shift_factor = predict_nr & 0xf;
	predict_nr >>= 4;
	flags = *(uint8*)pVAGBuffer++;	// flags

	for(i = 0; i < XA_BLOCK_SIZE; i += 2)
	{
		d = *(uint8*)pVAGBuffer++;
		s = (d & 0xf) << 12;
		if(s & 0x8000)
			s |= 0xffff0000;
		samples[i] = (float)(s >> shift_factor);
		s = (d & 0xf0) << 8;
		if(s & 0x8000)
			s |= 0xffff0000;
		samples[i+1] = (float)(s >> shift_factor);
	}

	for(i = 0; i < XA_BLOCK_SIZE; i++)
	{
		samples[i] = samples[i] + s_1 * f[predict_nr][0] + s_2 * f[predict_nr][1];
		s_2 = s_1;
		s_1 = samples[i];
		d = (int)(samples[i] + 0.5);
		*pOutputBuffer = d;
		++pOutputBuffer;
	}

	return flags;
}


void LoadVGSFile(GHSong *pSong, const char *pFilename)
{
	char *pBuffer = MFFileSystem_Load(MFStr("%s.vgs", pFilename));

	if(pBuffer)
	{
		VGSFile *pHeader = (VGSFile*)pBuffer;

		MFDebug_Assert(pHeader->VgS = MFMAKEFOURCC('V', 'g', 'S', '!'), "Not a VGS file!");
		char *pVGSData = (char*)&pHeader[1];

		int numChannels = 0;
		for(int a=0; a<8; a++)
		{
			if(pHeader->tracks[a].numBlocks)
				numChannels = a+1;
		}

		if(numChannels > 0)
			pSong->pVAGStream = MFSound_CreateDynamic(pFilename, XA_BLOCK_SIZE*pHeader->tracks[0].numBlocks, 2, 16, 44100);
		if(numChannels > 2)
			pSong->pVAGGuitar = MFSound_CreateDynamic(MFStr("%s-Guitar", pFilename), XA_BLOCK_SIZE*pHeader->tracks[0].numBlocks, 2, 16, 44100);
		if(numChannels > 4)
			pSong->pVAGBass = MFSound_CreateDynamic(MFStr("%s-Bass", pFilename), XA_BLOCK_SIZE*pHeader->tracks[0].numBlocks, 2, 16, 44100);

		struct sample
		{
			int16 left, right;
		};

		sample *pSampleData = NULL;
		sample *pGuitarData = NULL;
		sample *pBassData = NULL;
		uint32 len;

		if(numChannels > 0)
			MFSound_LockDynamic(pSong->pVAGStream, 0, 0, (void**)&pSampleData, &len);
		if(numChannels > 2)
			MFSound_LockDynamic(pSong->pVAGGuitar, 0, 0, (void**)&pGuitarData, &len);
		if(numChannels > 4)
			MFSound_LockDynamic(pSong->pVAGBass, 0, 0, (void**)&pBassData, &len);

		int16 left[XA_BLOCK_SIZE];
		int16 right[XA_BLOCK_SIZE];
		int16 leftGuitar[XA_BLOCK_SIZE];
		int16 rightGuitar[XA_BLOCK_SIZE];
		int16 leftBass[XA_BLOCK_SIZE];
		int16 rightBass[XA_BLOCK_SIZE];

		MFZeroMemory(left, sizeof(left));
		MFZeroMemory(right, sizeof(right));
		MFZeroMemory(leftGuitar, sizeof(leftGuitar));
		MFZeroMemory(rightGuitar, sizeof(rightGuitar));
		MFZeroMemory(leftBass, sizeof(leftBass));
		MFZeroMemory(rightBass, sizeof(rightBass));

		for(int a=0; a<pHeader->tracks[0].numBlocks; ++a)
		{
			for(int b=0; b<numChannels; ++b)
			{
				if(b == 0 && pSampleData)
				{
					int flags = UnpackVAGBlock(left, pVGSData);
				}
				else if(b == 1 && pSampleData)
				{
					int flags = UnpackVAGBlock(right, pVGSData);
				}
				else if(b == 2 && pGuitarData)
				{
					int flags = UnpackVAGBlock(leftGuitar, pVGSData);
				}
				else if(b == 3 && pGuitarData)
				{
					int flags = UnpackVAGBlock(rightGuitar, pVGSData);
				}
				else if(b == 4 && pBassData)
				{
					int flags = UnpackVAGBlock(leftGuitar, pVGSData);
				}
				else if(b == 5 && pBassData)
				{
					int flags = UnpackVAGBlock(rightGuitar, pVGSData);
				}

				pVGSData += XA_BLOCK_BYTES;
			}

			// write samples into buffer
			if(pSampleData)
			{
				for(int b=0; b<XA_BLOCK_SIZE; ++b)
				{
					pSampleData[b].left = left[b];
					pSampleData[b].right = right[b];
				}

				pSampleData += XA_BLOCK_SIZE;
			}

			if(pGuitarData)
			{
				for(int b=0; b<XA_BLOCK_SIZE; ++b)
				{
					pGuitarData[b].left = leftGuitar[b];
					pGuitarData[b].right = rightGuitar[b];
				}

				pGuitarData += XA_BLOCK_SIZE;
			}

			if(pBassData)
			{
				for(int b=0; b<XA_BLOCK_SIZE; ++b)
				{
					pBassData[b].left = leftBass[b];
					pBassData[b].right = rightBass[b];
				}

				pBassData += XA_BLOCK_SIZE;
			}
		}

		if(numChannels > 0)
			MFSound_UnlockDynamic(pSong->pVAGStream);
		if(numChannels > 2)
			MFSound_UnlockDynamic(pSong->pVAGGuitar);
		if(numChannels > 4)
			MFSound_UnlockDynamic(pSong->pVAGBass);

		MFHeap_Free(pBuffer);
	}
}

/*

PSX VAG-Packer, hacked by bITmASTER@bigfoot.com
v0.1                             
*/


/*
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <conio.h>

#define BUFFER_SIZE 128*XA_BLOCK_SIZE

short wave[BUFFER_SIZE];

void find_predict( short *samples, double *d_samples, int *predict_nr, int *shift_factor );
void pack( double *d_samples, short *four_bit, int predict_nr, int shift_factor );
void fputi( int d, FILE *fp );

int main( int argc, char *argv[] )
{
	FILE *fp, *vag;
	char fname[128];
	char *p;
	short *ptr;
	double d_samples[28];
	short four_bit[28];
	int predict_nr;
	int shift_factor;
	int flags;
	int size;
	int i, j, k;   
	unsigned char d;
	char s[4];
	int chunk_data;
	short e;
	int sample_freq, sample_len;

	if ( argc != 2 ) {
		printf( "usage: vag-pack *.wav\n" );
		return( -1 );
	}

	fp = fopen( argv[1], "rb" );
	if ( fp == NULL ) {
		printf( "canｴt open %s\n", argv[1] );
		return( -2 );
	}

	fread( s, 1, 4, fp );
	if ( strncmp( s, "RIFF", 4 ) ) {
		printf( "not a wav-file\n" );
		return( -3 );
	}

	fseek( fp, 8, SEEK_SET );
	fread( s, 1, 4, fp );
	if ( strncmp( s, "WAVE", 4 ) ) {
		printf( "not a wav-file\n" );
		return( -3 );
	}

	fseek( fp, 8 + 4, SEEK_SET );
	fread( s, 1, 4, fp );
	if ( strncmp( s, "fmt", 3 ) ) {
		printf( "not a wav-file\n" );
		return( -3 );
	}

	fread( &chunk_data, 4, 1, fp );
	chunk_data += ftell( fp );

	fread( &e, 2, 1, fp );
	if ( e != 1 ) {
		printf( "no PCM\n" );
		return( -4 );
	} 

	fread( &e, 2, 1, fp );
	if ( e != 1 ) {
		printf( "must be MONO\n" );
		return( -5 );
	}

	fread( &sample_freq, 4, 1, fp );
	fseek( fp, 4 + 2, SEEK_CUR );

	fread( &e, 2, 1, fp );
	if ( e != 16 ) {
		printf( "only 16 bit samples\n" );
		return( -6 );
	}     

	fseek( fp, chunk_data, SEEK_SET );

	fread( s, 1, 4, fp );
	if ( strncmp( s, "data", 4 ) ) {
		printf( "no data chunk \n" );
		return( -7 );
	}

	fread( &sample_len, 4, 1, fp );
	sample_len /= 2;

	strcpy( fname, argv[1] );
	p = strrchr( fname, '.' );
	p++;
	strcpy( p, "VAG" );
	vag = fopen( fname, "wb" );
	if ( vag == NULL ) {
		printf( "canｴt write output file\n" );
		return( -8 );
	}

	fprintf( vag, "VAGp" );            // ID
	fputi( 0x20, vag );                // Version
	fputi( 0x00, vag );                // Reserved
	size = sample_len / 28;
	if( sample_len % 28 )
		size++;
	fputi( 16 * ( size + 2 ), vag );    // Data size
	fputi( sample_freq, vag );          // Sampling frequency

	for ( i = 0; i < 12; i++ )          // Reserved
		fputc( 0, vag );

	p -= 2;
	i = 0;
	while( isalnum( *p ) ) {
		i++;
		p--;
	}
	p++;
	for ( j = 0; j < i; j++ )          // Name
		fputc( *p++, vag );
	for( j = 0; j < 16-i; j++ )
		fputc( 0, vag );

	for( i = 0; i < 16; i++ )
		fputc( 0, vag );                // ???

	flags = 0; 
	while( sample_len > 0 ) {
		size = ( sample_len >= BUFFER_SIZE ) ? BUFFER_SIZE : sample_len;
		fread( wave, sizeof( short ), size, fp );
		i = size / 28;
		if ( size % 28 ) {
			for ( j = size % 28; j < 28; j++ )
				wave[28*i+j] = 0;
			i++;
		}

		for ( j = 0; j < i; j++ ) {                                    // pack 28 samples
			ptr = wave + j * 28;
			find_predict( ptr, d_samples, &predict_nr, &shift_factor );
			pack( d_samples, four_bit, predict_nr, shift_factor );
			d = ( predict_nr << 4 ) | shift_factor;
			fputc( d, vag );
			fputc( flags, vag );
			for ( k = 0; k < 28; k += 2 ) {
				d = ( ( four_bit[k+1] >> 8 ) & 0xf0 ) | ( ( four_bit[k] >> 12 ) & 0xf );
				fputc( d, vag );
			}
			sample_len -= 28;
			if ( sample_len < 28 )
				flags = 1;
		}
	}

	fputc( ( predict_nr << 4 ) | shift_factor, vag );
	fputc( 7, vag );            // end flag
	for ( i = 0; i < 14; i++ )
		fputc( 0, vag );


	fclose( fp );
	fclose( vag ); 
	//    getch();
	return( 0 );
}


static double f[5][2] = { { 0.0, 0.0 },
{  -60.0 / 64.0, 0.0 },
{ -115.0 / 64.0, 52.0 / 64.0 },
{  -98.0 / 64.0, 55.0 / 64.0 },
{ -122.0 / 64.0, 60.0 / 64.0 } };



void find_predict( short *samples, double *d_samples, int *predict_nr, int *shift_factor )
{
	int i, j;
	double buffer[28][5];
	double min = 1e10;
	double max[5];
	double ds;
	int min2;
	int shift_mask;
	static double _s_1 = 0.0;                            // s[t-1]
	static double _s_2 = 0.0;                            // s[t-2]
	double s_0, s_1, s_2;

	for ( i = 0; i < 5; i++ ) {
		max[i] = 0.0;
		s_1 = _s_1;
		s_2 = _s_2;
		for ( j = 0; j < 28; j ++ ) {
			s_0 = (double) samples[j];                      // s[t-0]
			if ( s_0 > 30719.0 )
				s_0 = 30719.0;
			if ( s_0 < - 30720.0 )
				s_0 = -30720.0;
			ds = s_0 + s_1 * f[i][0] + s_2 * f[i][1];
			buffer[j][i] = ds;
			if ( fabs( ds ) > max[i] )
				max[i] = fabs( ds );
			//                printf( "%+5.2f\n", s2 );
			s_2 = s_1;                                  // new s[t-2]
			s_1 = s_0;                                  // new s[t-1]
		}

		if ( max[i] < min ) {
			min = max[i];
			*predict_nr = i;
		}
		if ( min <= 7 ) {
			*predict_nr = 0;
			break;
		}

	}

	// store s[t-2] and s[t-1] in a static variable
	// these than used in the next function call

	_s_1 = s_1;
	_s_2 = s_2;

	for ( i = 0; i < 28; i++ )
		d_samples[i] = buffer[i][*predict_nr];

	//  if ( min > 32767.0 )
	//      min = 32767.0;

	min2 = ( int ) min;
	shift_mask = 0x4000;
	*shift_factor = 0;

	while( *shift_factor < 12 ) {
		if ( shift_mask  & ( min2 + ( shift_mask >> 3 ) ) )
			break;
		(*shift_factor)++;
		shift_mask = shift_mask >> 1;
	}

}

void pack( double *d_samples, short *four_bit, int predict_nr, int shift_factor )
{
	double ds;
	int di;
	double s_0;
	static double s_1 = 0.0;
	static double s_2 = 0.0;
	int i;

	for ( i = 0; i < 28; i++ ) {
		s_0 = d_samples[i] + s_1 * f[predict_nr][0] + s_2 * f[predict_nr][1];
		ds = s_0 * (double) ( 1 << shift_factor );

		di = ( (int) ds + 0x800 ) & 0xfffff000;

		if ( di > 32767 )
			di = 32767;
		if ( di < -32768 )
			di = -32768;

		four_bit[i] = (short) di;

		di = di >> shift_factor;
		s_2 = s_1;
		s_1 = (double) di - s_0;

	}
}

void fputi( int d, FILE *fp )
{
	fputc( d >> 24, fp );
	fputc( d >> 16, fp ); 
	fputc( d >> 8,  fp );
	fputc( d,      fp );
}
*/
#endif