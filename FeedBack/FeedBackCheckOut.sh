mkdir Fuji
mkdir Utilities
mkdir FeedBack
mkdir FeedBack_Data
mkdir FeedBack_Assets

svn co svn://db.bounceme.net/Fuji Fuji/
svn co svn://db.bounceme.net/Utilities Utilities/
svn co svn://db.bounceme.net/FeedBack FeedBack/
svn co svn://db.bounceme.net/FeedBack_Data FeedBack_Data/
svn co svn://db.bounceme.net/FeedBack_Assets FeedBack_Assets/

cd Fuji
make tools

cd ../FeedBack
make rebuild
