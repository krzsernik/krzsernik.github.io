#include <iostream>
#include <fstream>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

extern "C" {
	int setImage(int index, const char *str);
	int buildLogo();
	const char *getLastError();
	// int saveBMPData(const char *filename);
}

using namespace std;

const int ADDRESSES[] = {
	0x4000,
	0x5000,
	0x740000,
	0xd2f000,
	0x146a000
};
const int IMAGE_SIZE = 0x1CF5000;
const unsigned char SIGNATURE[] = {
	0X4C, 0X4F, 0X47, 0X4F, 0X21, 0X21, 0X21, 0X21,  0X5,  0X0, 
	  0X0,  0X0, 0X3B,  0X7,  0X0,  0X0, 0X40,  0X7,  0X0,  0X0, 
	 0XEF,  0X5,  0X0,  0X0, 0X2F,  0XD,  0X0,  0X0, 0X3B,  0X7,
	  0X0,  0X0, 0X6A, 0X14,  0X0,  0X0, 0XEF,  0X5
};
const int SIGNATURE_LENGTH = 38;
const char *DEFAULT_FILE_NAMES[] = {
	"locked.bmp",
	"fastboot.bmp",
	"unlocked.bmp",
	"dead.bmp"
};
const char *CUSTOM_FILE_NAMES[] = {
	"c_locked.bmp",
	"c_fastboot.bmp",
	"c_unlocked.bmp",
	"c_dead.bmp"
};

unsigned char *logo_img = new unsigned char[IMAGE_SIZE];

int buildLogo() {
	// set memory of logo.img to 0's
	memset(logo_img, 0, IMAGE_SIZE);

	for(int img=0; img < 4; img++) {
		ifstream f(CUSTOM_FILE_NAMES[img], ios::binary);

		if(!f.is_open()) {
			f.close();
			f.open(DEFAULT_FILE_NAMES[img], ios::binary);
		}

		// get size of custom image
		int size = f.tellg();
		f.seekg(0, ios::end);
		size = (int)f.tellg() - size;
		f.seekg(0);

		f.read((char*)(logo_img + ADDRESSES[img + 1]), size);
		printf("Wrote %i\n", size);

		f.close();
	}

	for(int i=0; i < SIGNATURE_LENGTH; i++) {
		logo_img[ADDRESSES[0] + i] = SIGNATURE[i];
	}

	// save as logo.img to allow JS to download file
	ofstream logo("logo.img", ios::binary);
	logo.write((char*)logo_img, IMAGE_SIZE);
	logo.close();

	return 0;
}

int lastError = -1;
const char *ERRORS[] = {
	"File corrupted or invalid. Supported files: PNG, JPG, BMP, GIF (non-animated), HDR, PSD, TGA, PIC, PGM, PPM.",
	"Image too big. Max dimensions are 1080x2340.",
};

const char *getLastError(){
	if(lastError == -1) return NULL;

	const char *error = ERRORS[lastError];
	printf("%s\n", error);
	lastError = -1;
	printf("%s\n", error);

	return error;
}

int saveBMPData(int index, const char *filename) {
	int w, h, n;
    unsigned char *data = stbi_load(filename, &w, &h, &n, 3);

    if(data == NULL) {
    	lastError = 0;
    	return -1;
    }

    if(w > 1080 || h > 2340) {
    	lastError = 1;
    	return -1;
    }

    // BMP writing by deusmacabre, edited by me
    // https://stackoverflow.com/questions/2654480/writing-bmp-image-in-pure-c-c-without-other-libraries
    unsigned char bmpfileheader[14] = {'B','M', 0,0,0,0, 0,0, 0,0, 54,0,0,0};
	unsigned char bmpinfoheader[40] = {40,0,0,0, 0,0,0,0, 0,0,0,0, 1,0, 24,0};
    int w1 = ((4-(w*3)%4)%4);
    int filesize = 54 + (3 * w + w1) * h;

    bmpfileheader[ 2] = (unsigned char)(filesize    );
	bmpfileheader[ 3] = (unsigned char)(filesize>> 8);
	bmpfileheader[ 4] = (unsigned char)(filesize>>16);
	bmpfileheader[ 5] = (unsigned char)(filesize>>24);

	bmpinfoheader[ 4] = (unsigned char)(w    );
	bmpinfoheader[ 5] = (unsigned char)(w>> 8);
	bmpinfoheader[ 6] = (unsigned char)(w>>16);
	bmpinfoheader[ 7] = (unsigned char)(w>>24);
	bmpinfoheader[ 8] = (unsigned char)(h    );
	bmpinfoheader[ 9] = (unsigned char)(h>> 8);
	bmpinfoheader[10] = (unsigned char)(h>>16);
	bmpinfoheader[11] = (unsigned char)(h>>24);

    unsigned char *img = (unsigned char *)malloc(filesize);
    memset(img, 0, filesize);
    memcpy(img, bmpfileheader, 14);
    memcpy(img+14, bmpinfoheader, 40);

    int x, y, p;
    for(int i=0; i<w; i++) {
	    for(int j=0; j<h; j++) {
	        x=i; 
	        y=(h-1)-j;
	        p = 54+(x+y*w)*3+w1*y;

	        // img[p+2] = data[(i+j*w)*3+0];
	        // img[p+1] = data[(i+j*w)*3+1];
	        // img[p+0] = data[(i+j*w)*3+2];

	        // reverse_copy does the same thing
	        reverse_copy(data+(i+j*w)*3+0, data+(i+j*w)*3+3, img+p);
	    }
	}
    // memcpy(img+54, data, filesize - 54);

	// free memory used by image data
    stbi_image_free(data);
    // remove file from emulated filesystem
    remove(filename);

    // write our bmp as custom file
    ofstream f(CUSTOM_FILE_NAMES[index], ios::binary);
    f.write((char*)img, filesize);
    f.close();

    return 0;
}

int setImage(int index, const char *str) {
	bool def = false;

	if(strlen(str) == 0) {
		def = true;
	} else {
		for(int i = 0; i < 4; i++) {
			if(strcmp(str, DEFAULT_FILE_NAMES[i]) == 0) def = true;
		}
	}

	if(def) {
		return remove(CUSTOM_FILE_NAMES[index]);
	}

	return saveBMPData(index, str);
}

int main() {
	printf("C++ loaded.\n");

	return 0;
}

