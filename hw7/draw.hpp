#include <iostream>
#include <cmath>

const int BYTES_PER_PIXEL = 3; /// red, green, & blue
const int FILE_HEADER_SIZE = 14;
const int INFO_HEADER_SIZE = 40;

using pos = std::pair<int,int>;
using dpos = std::pair<double,double>;

using byte=unsigned char;

#define B 0
#define G 1
#define R 2

struct color{
    byte b,g,r;


    byte operator[](int index) const{
        switch (index) {
            case R:
                return r;
            case G:
                return g;
            case B:
                return b;
            default:
                throw "Invalid color";
        }
    }
};

dpos normalize(dpos p){
    if(p.first ==0 && p.second ==0){
        return {0,0};
    }
    double sz = sqrt(p.first*p.first + p.second*p.second);
    return {p.first/sz,p.second/sz};
}

double dist(dpos u, dpos v){
    double x = u.first-v.first;
    double y = u.second-v.second;
    return sqrt(x*x + y*y);
}

struct img{
    color** image;
    int width,height;

    img(int width,int height,color dColor):width(width),height(height){
        image = new color*[height];
        for(auto i=0;i<height;i++){
            image[i] = new color[width];
            for(auto j=0;j<width;j++){
                image[i][j]= dColor;
            }
        }
    }

    pos transform(dpos pp) const{
        if(pp.first <0 || pp.first > 1){
            throw "Out of bounds";
        }
        if(pp.second <0 || pp.second > 1){
            throw "Out of bounds";
        }

        return {pp.first*(width-1),pp.second*(height-1)};
    }
    void draw(dpos pos,color c,double size=.005){
        dpos dminp = {std::max(pos.first-size,0.0),std::max(pos.second-size,0.0)};
        dpos dmaxp ={std::min(pos.first+size,1.0),std::min(pos.second+size,1.0)};

        auto minp = transform(dminp);
        auto maxp = transform(dmaxp);

        for(auto i=minp.first;i<=maxp.first;i++){
            for(auto j=minp.second;j<=maxp.second;j++){
                if(dist({i/(double)(width-1),j/(double)(height-1)},pos) <= size){
                    image[j][i] = c;
                }
            }
        }
    }

    ~img(){
        for(auto i=0;i<height;i++){
            delete[] image[i];
        }

        delete[] image;
    }
};

byte* createBitmapFileHeader (int height, int stride)
{
    int fileSize = FILE_HEADER_SIZE + INFO_HEADER_SIZE + (stride * height);

    static byte fileHeader[] = {
            0,0,     /// signature
            0,0,0,0, /// image file size in bytes
            0,0,0,0, /// reserved
            0,0,0,0, /// start of pixel array
    };

    fileHeader[ 0] = (byte)('B');
    fileHeader[ 1] = (byte)('M');
    fileHeader[ 2] = (byte)(fileSize);
    fileHeader[ 3] = (byte)(fileSize >>  8);
    fileHeader[ 4] = (byte)(fileSize >> 16);
    fileHeader[ 5] = (byte)(fileSize >> 24);
    fileHeader[10] = (byte)(FILE_HEADER_SIZE + INFO_HEADER_SIZE);

    return fileHeader;
}
byte* createBitmapInfoHeader (int height, int width)
{
    static byte infoHeader[] = {
            0,0,0,0, /// header size
            0,0,0,0, /// image width
            0,0,0,0, /// image height
            0,0,     /// number of color planes
            0,0,     /// bits per pixel
            0,0,0,0, /// compression
            0,0,0,0, /// image size
            0,0,0,0, /// horizontal resolution
            0,0,0,0, /// vertical resolution
            0,0,0,0, /// colors in color table
            0,0,0,0, /// important color count
    };

    infoHeader[ 0] = (byte)(INFO_HEADER_SIZE);
    infoHeader[ 4] = (byte)(width      );
    infoHeader[ 5] = (byte)(width >>  8);
    infoHeader[ 6] = (byte)(width >> 16);
    infoHeader[ 7] = (byte)(width >> 24);
    infoHeader[ 8] = (byte)(height      );
    infoHeader[ 9] = (byte)(height >>  8);
    infoHeader[10] = (byte)(height >> 16);
    infoHeader[11] = (byte)(height >> 24);
    infoHeader[12] = (byte)(1);
    infoHeader[14] = (byte)(BYTES_PER_PIXEL*8);

    return infoHeader;
}
void generateBitmapImage (const img& data,const char* imageFileName)
{
    int widthInBytes = data.width * BYTES_PER_PIXEL;

    byte padding[3] = {0, 0, 0};
    int paddingSize = (4 - (widthInBytes) % 4) % 4;

    int stride = (widthInBytes) + paddingSize;

    FILE* imageFile = fopen(imageFileName, "wb");

    byte* fileHeader = createBitmapFileHeader(data.height, stride);
    fwrite(fileHeader, 1, FILE_HEADER_SIZE, imageFile);

    byte* infoHeader = createBitmapInfoHeader(data.height, data.width);
    fwrite(infoHeader, 1, INFO_HEADER_SIZE, imageFile);

    for (auto i = 0; i < data.height; i++) {
        fwrite(data.image[i], BYTES_PER_PIXEL, data.width, imageFile);
        fwrite(padding, 1, paddingSize, imageFile);
    }

    fclose(imageFile);
}


