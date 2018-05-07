#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef struct {
     unsigned char red,green,blue;
} ImageFormatPixel;

typedef struct {
     int x, y;
     ImageFormatPixel *data;
} FormatImage;

#define RGB_COLOR 255

FormatImage *read_PPM_file(char *);
void encrypt_hidden_image(char *, FormatImage *, FormatImage *);
void decrypt_hidden_image(FormatImage *);
unsigned char add(unsigned char, unsigned char);
unsigned char get_truncated_value(unsigned char);
unsigned char get_two_bits(unsigned char, int, int);
unsigned short get_short_two_bits(unsigned short, int, int);
unsigned short shift_left(unsigned short, int);
unsigned char combine_four_two_bits_data(unsigned char, unsigned char, unsigned char, unsigned char);
unsigned short combine_eight_two_bits_data(unsigned short, unsigned short, unsigned short, unsigned short, 
										unsigned short, unsigned short, unsigned short, unsigned short);

int main(int argc, char **argv) {

    printf("Enter Y to encrypt the hidden image, N to decrypt it, else to exit: ");
    char ch = getchar();
    getchar();

	if(ch == 'Y'){

		char first[100];
		char second[100];

	    printf("Enter the first image hide: ");
	    fgets(first, 100, stdin);
	    strtok(first, "\n");

	    FormatImage *image1 = read_PPM_file(first);

	    printf("Enter the image you want to hide: ");
	    fgets(second, 100, stdin);
	    strtok(second, "\n");
		
		FormatImage *image2 = read_PPM_file(second);

		encrypt_hidden_image("encrypted.ppm", image1, image2);
		printf("Encryption is done\n");

		free(image1);
		free(image2);

	}else if(ch == 'N') {

        char encrypted_image_name[100];

		printf("Enter the encrypted image: ");
		fgets(encrypted_image_name, 100, stdin);
		strtok(encrypted_image_name, "\n");

		FormatImage *modified_image = read_PPM_file(encrypted_image_name);
		
		decrypt_hidden_image(modified_image);
		printf("Decryption is done\n");

		free(modified_image);
	}else {
		printf("Program has been terminated.\n");
		exit(0);
	}

	return 0;
}

FormatImage *read_PPM_file(char *file) {

	int c, rgb_value;
	char line[10];
	FILE *fp;
	FormatImage *image;

	if((fp = fopen(file, "r")) == NULL) {
		printf("%s cannot be opened or it does not exist\n", file);
		exit(1);
	}

	if((fgets(line, sizeof(line), fp)) == NULL) {
		printf("file lines cannot be read\n");
		exit(1);
	}

	if(line[0] != 'P' && line[1] != '6') {
		printf("Image file must be PPM format\n");
		exit(1);
	}

	if((image = (FormatImage*)malloc(sizeof(FormatImage))) == NULL) {
		printf("Cannot allocate memory for the image");
		exit(1);
	}

	c = getc(fp);
    while (c == '#') {
        while (getc(fp) != '\n')
            ;
            c = getc(fp);
    }
    ungetc(c, fp);

    if(fscanf(fp, "%d %d", &image->x, &image->y) != 2) {
    	printf("Invalid image size\n");
    	exit(1);
    }

    if(fscanf(fp, "%d", &rgb_value) != 1) {
    	printf("Invalid RGB value\n");
    	exit(1);
    } else {
    	if(rgb_value != RGB_COLOR) {
    		printf("Image file must be 8-bits color\n");
    		exit(1);
    	}
    }

    while (fgetc(fp) != '\n') 
        ;

    if((image->data = (ImageFormatPixel*)malloc(image->x * image->y * sizeof(ImageFormatPixel))) == NULL) {
    	printf("Cannot allocate memory for image data\n");
    	exit(1);
    }

    if (fread(image->data, 3 * image->x, image->y, fp) != image->y) {
         printf("Error loading image");
         exit(1);
    }

    fclose(fp);

    return image;
}

void encrypt_hidden_image(char *modified_filename, FormatImage *first_image, FormatImage *second_image) {

	FILE *fp;
	FormatImage *image;

	int k = 0, c = 0, i = 0;

	if((second_image->x * second_image->y) > (((first_image->x * first_image->y) / 4) - 6)) {
		printf("Second image cannot be bigger than the first image\n");
		exit(1);
	}

	if((image = (FormatImage*)malloc(sizeof(FormatImage))) == NULL) {
		printf("Unable to allocate memory\n");
		exit(1);
	}

	if((image->data = (ImageFormatPixel*)malloc(first_image->x * first_image->y * sizeof(ImageFormatPixel))) == NULL) {
		printf("Unable to allocate memory for image data\n");
		exit(1);
	}

	if((fp = fopen(modified_filename, "wb")) == NULL) {
		printf("Unable to open this file\n");
		exit(1);
	}

	fprintf(fp, "P6\n");	// put the ppm file header to the new file
	fprintf(fp, "%d %d\n", first_image->x, first_image->y);	// put the width and length of pixel to the new file
	fprintf(fp, "%d\n", RGB_COLOR);	// put the color bits value to the new file

	int *color_channel;

	if((color_channel = (int*)malloc(sizeof(int) * 3)) == NULL) {
		printf("Cannot allocate memory for color color_channel\n");
		exit(1);
	}

	int left = 0, right = 14;
	for(i = 0; i < 3; i++) {
		image->data[i].red = add(get_truncated_value(first_image->data[i].red), get_short_two_bits(second_image->x, left, right));
		left += 2;
		image->data[i].green = add(get_truncated_value(first_image->data[i].green), get_short_two_bits(second_image->x, left, right));
		left += 2;
		if(i != 2) {
			image->data[i].blue = add(get_truncated_value(first_image->data[i].blue), get_short_two_bits(second_image->x, left, right));
			left += 2;
		}else{
			left = 0;
		}
	}

	while(i < 6) {
		image->data[i].red = add(get_truncated_value(first_image->data[i].red), get_short_two_bits(second_image->y, left, right));
		left += 2;
		image->data[i].green = add(get_truncated_value(first_image->data[i].green), get_short_two_bits(second_image->y, left, right));
		left += 2;
		if(i != 5) {
			image->data[i].blue = add(get_truncated_value(first_image->data[i].blue), get_short_two_bits(second_image->y, left, right));
			left += 2;
		}
		i++;
	}

	left = 0;
	right = 6;

	color_channel[0] = (int)second_image->data[c].red;
	color_channel[1] = (int)second_image->data[c].green;
	color_channel[2] = (int)second_image->data[c].blue;

	int len = second_image->x * second_image->y;

	for(i = 6; i < first_image->x * first_image->y ; i++) {
		if(c < len) {
			image->data[i].red = add(get_truncated_value(first_image->data[i].red), get_two_bits(color_channel[k], left, right));
			left += 2;
		}else{
			image->data[i].red = first_image->data[i].red;
		}
		if(left == 8) {
			left = 0;
			k++;
			if(k == 3) {
				k = 0;
				color_channel[0] = (int)second_image->data[++c].red;
				color_channel[1] = (int)second_image->data[c].green;
				color_channel[2] = (int)second_image->data[c].blue;
			}
		}

		if(c < len) {
			image->data[i].green = add(get_truncated_value(first_image->data[i].green), get_two_bits(color_channel[k], left, right));
			left += 2;
		}else{
			image->data[i].green = first_image->data[i].green;
		}
		if(left == 8) {
			left = 0;
			k++;
			if(k == 3) {
				k = 0;
				color_channel[0] = (int)second_image->data[++c].red;
				color_channel[1] = (int)second_image->data[c].green;
				color_channel[2] = (int)second_image->data[c].blue;
			}
		}

		if(c < len) {
			image->data[i].blue = add(get_truncated_value(first_image->data[i].blue), get_two_bits(color_channel[k], left, right));
			left += 2;
		}else{
			image->data[i].blue = first_image->data[i].blue;
		}
		if(left == 8) {
			left = 0;
			k++;
			if(k == 3) {
				k = 0;
				color_channel[0] = (int)second_image->data[++c].red;
				color_channel[1] = (int)second_image->data[c].green;
				color_channel[2] = (int)second_image->data[c].blue;
			}
		}
	}

	fwrite(image->data, sizeof(ImageFormatPixel), first_image->x * first_image->y, fp);

	free(color_channel);
	free(image);
	fclose(fp);

}

void decrypt_hidden_image(FormatImage* image) {

	FormatImage *hidden_image;
	FILE *fp;
	int i = 0, offset = 14, j = 0;

	unsigned short r1 = get_short_two_bits(image->data[i].red, offset, offset);
	unsigned short g1 = get_short_two_bits(image->data[i].green, offset, offset);
	unsigned short b1 = get_short_two_bits(image->data[i++].blue, offset, offset);
	unsigned short r2 = get_short_two_bits(image->data[i].red, offset, offset);
	unsigned short g2 = get_short_two_bits(image->data[i].green, offset, offset);
	unsigned short b2 = get_short_two_bits(image->data[i++].blue, offset, offset);
	unsigned short r3 = get_short_two_bits(image->data[i].red, offset, offset);
	unsigned short g3 = get_short_two_bits(image->data[i++].green, offset, offset);

	unsigned short width = combine_eight_two_bits_data(r1, g1, b1, r2, g2, b2, r3, g3);

	unsigned short r4 = get_short_two_bits(image->data[i].red, offset, offset);
	unsigned short g4 = get_short_two_bits(image->data[i].green, offset, offset);
	unsigned short b4 = get_short_two_bits(image->data[i++].blue, offset, offset);
	unsigned short r5 = get_short_two_bits(image->data[i].red, offset, offset);
	unsigned short g5 = get_short_two_bits(image->data[i].green, offset, offset);
	unsigned short b5 = get_short_two_bits(image->data[i++].blue, offset, offset);
	unsigned short r6 = get_short_two_bits(image->data[i].red, offset, offset);
	unsigned short g6 = get_short_two_bits(image->data[i++].green, offset, offset);

	unsigned short height = combine_eight_two_bits_data(r4, g4, b4, r5, g5, b5, r6, g6);

	if((fp = fopen("hidden_image.ppm", "wb")) == NULL) {
		printf("Unable to create file\n");
		exit(1);
	}

	if((hidden_image = (FormatImage*)malloc(sizeof(FormatImage))) == NULL) {
		printf("Unable to allocate memory for the hidden image\n");
		exit(1);
	}

	if((hidden_image->data = (ImageFormatPixel*)malloc(width * height * sizeof(ImageFormatPixel))) == NULL) {
		printf("Unable to allocate memory for image contents\n");
		exit(1);
	}

	fprintf(fp, "P6\n");	// put the ppm file header to the new file
	fprintf(fp, "%d %d\n", (int)width, (int)height);	// put the width and length of pixel to the new file
	fprintf(fp, "%d\n", RGB_COLOR);	// put the color bits value to the new file

	offset = 6;

	printf("Entering the loop\n");

	for(i = 0, j = 6; i < width * height; i++, j += 4) {

		unsigned char r1 = get_two_bits(image->data[j].red, offset, offset);
		unsigned char g1 = get_two_bits(image->data[j].green, offset, offset);
		unsigned char b1 = get_two_bits(image->data[j].blue, offset, offset);
		unsigned char r2 = get_two_bits(image->data[j + 1].red, offset, offset);
		hidden_image->data[i].red = combine_four_two_bits_data(r1, g1, b1, r2);

		unsigned char g2 = get_two_bits(image->data[j + 1].green, offset, offset);
		unsigned char b2 = get_two_bits(image->data[j + 1].blue, offset, offset);
		unsigned char r3 = get_two_bits(image->data[j + 2].red, offset, offset);
		unsigned char g3 = get_two_bits(image->data[j + 2].green, offset, offset);
		hidden_image->data[i].green = combine_four_two_bits_data(g2, b2, r3, g3);

		unsigned char b3 = get_two_bits(image->data[j + 2].blue, offset, offset);
		unsigned char r4 = get_two_bits(image->data[j + 3].red, offset, offset);
		unsigned char g4 = get_two_bits(image->data[j + 3].green, offset, offset);
		unsigned char b4 = get_two_bits(image->data[j + 3].blue, offset, offset);
		hidden_image->data[i].blue = combine_four_two_bits_data(b3, r4, g4, b4);
	}

	fwrite(hidden_image->data, sizeof(ImageFormatPixel), width * height, fp);

	free(hidden_image);
	fclose(fp);

}

unsigned char add(unsigned char x, unsigned char y) {
    while (y != 0) {
        int carry = x & y;
        x = x ^ y;
        y = carry << 1;
    }
    return x;
}

unsigned char get_truncated_value(unsigned char input) {
	return (input >> 2) << 2;
}

unsigned char get_two_bits(unsigned char input, int left, int right) {
	return (unsigned char)(input << left) >> right;
}

unsigned short get_short_two_bits(unsigned short input, int left, int right) {
	return (unsigned short)(input << left) >> right;
}

unsigned short shift_left(unsigned short input, int offset) {
	return (unsigned short)input << offset;
}

unsigned char combine_four_two_bits_data(unsigned char data1, unsigned char data2, unsigned char data3, unsigned char data4) {
	return (unsigned char)(shift_left(data1, 6) | shift_left(data2, 4) | shift_left(data3, 2) | data4);
}

unsigned short combine_eight_two_bits_data(unsigned short data1, unsigned short data2, unsigned short data3, unsigned short data4, 
										unsigned short data5, unsigned short data6, unsigned short data7, unsigned short data8) {
	return (unsigned short)(shift_left(data1, 14) | shift_left(data2, 12) | shift_left(data3, 10) | shift_left(data4, 8)
							| shift_left(data5, 6) | shift_left(data6, 4) | shift_left(data7, 2) | data8);
}












