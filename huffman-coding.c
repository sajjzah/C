#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void strconcat(char* str, char* parentcode, char add)
{
    int i = 0;

    while (*(parentcode + i) != '\0')
    {
        *(str + i) = *(parentcode + i);
        i++;
    }

    str[i] = add;
    str[i + 1] = '\0';
}

int fib(int n)
{
    if (n <= 1)
        return n;
    return fib(n - 1) + fib(n - 2);
}

int main()
{
    int width, height, itemp = 0, bpp = 0;
    float ftemp = 0.0f;
    int **image;
    char filename[] = "D:\\SJ\\UMZ\\Term 4\\Algorithms Design\\project\\grey2.bmp";
    FILE *image_file;

    image_file = fopen(filename, "rb");

    if(image_file == NULL)
    {
        printf("Error opening file!\n\nPress enter to exit...");
        getchar();
        exit(1);
    }
    else
    {
        int data, offset, hbytes, bmpsize = 0, bmpdataoff = 0;

        printf("*** Process Image ***\n");

        offset = 0;
        fseek(image_file, offset, SEEK_SET);
        printf("- File ID: ");
        for(int i = 0; i < 2; i++)
        {
            fread(&data, 1, 1, image_file);
            printf("%c", data);
        }

        fread(&bmpsize, 4, 1, image_file);
        printf("\n- Size of the BMP File: %d bytes\n", bmpsize);

        offset = 10;
        fseek(image_file, offset, SEEK_SET);
        fread(&bmpdataoff, 4, 1, image_file);
        printf("- Bitmap data offset: %d\n", bmpdataoff);

        fread(&hbytes, 4, 1, image_file);
        printf("- Number of bytes in header: %d\n", hbytes);

        fread(&width, 4, 1, image_file);
        fread(&height, 4, 1, image_file);
        printf("- Width of Image: %d\n", width);
        printf("- Height of Image: %d\n", height);

        fseek(image_file, 2, SEEK_CUR);
        fread(&bpp, 2, 1, image_file);
        printf("- Number of bits per pixel: %d\n", bpp);

        fseek(image_file, bmpdataoff, SEEK_SET);

        image = (int **)malloc(height * sizeof(int *));
        for(int i = 0; i < height; i++)
        {
            image[i] = (int *)malloc(width * sizeof(int));
        }

        for(int i = 0; i < height; i++)
        {
            for(int j = 0; j < width; j++)
            {
                fread(&itemp, 1, 1, image_file);
                itemp = itemp & 0xFF;
                image[i][j] = itemp;
            }
        }
    }

    fclose(image_file);

    int hist[256];
    for (int i = 0; i < 256; i++)
        hist[i] = 0;
    for (int i = 0; i < height; i++)
        for (int j = 0; j < width; j++)
            hist[image[i][j]] += 1;

    int nodes = 0;
    for (int i = 0; i < 256; i++)
        if (hist[i] != 0)
            nodes += 1;

    float prob = 1.0;
    int totalpix = height * width;
    for (int i = 0; i < 256; i++)
    {
        ftemp = (hist[i] / (float)(totalpix));
        if (ftemp > 0 && ftemp <= prob)
            prob = ftemp;
    }

    itemp = 0;
    while ((1 / prob) > fib(itemp))
        itemp++;
    int maxcodelen = itemp - 3;

    struct pixfreq
    {
        int pix;
        float freq;
        struct pixfreq *left, *right;
        char code[maxcodelen];
    };

    struct huffcode
    {
        int pix, arrloc;
        float freq;
    };

    struct pixfreq * pix_freq;
    struct huffcode * huffcodes;

    int totalnodes = 2 * nodes - 1;

    pix_freq = (struct pixfreq*)malloc(sizeof(struct pixfreq) * totalnodes);
    huffcodes = (struct huffcode*)malloc(sizeof(struct huffcode) * nodes);

    for (int i = 0, j = 0; i < 256; i++)
    {
        if (hist[i] != 0)
        {
            huffcodes[j].pix = i;
            pix_freq[j].pix = i;

            huffcodes[j].arrloc = j;

            ftemp = (float)hist[i] / (float)totalpix;
            pix_freq[j].freq = ftemp;
            huffcodes[j].freq = ftemp;

            pix_freq[j].left = NULL;
            pix_freq[j].right = NULL;

            *(pix_freq[j].code) = '\0';

            j++;
        }
    }

    struct huffcode temphuff;

    for (int i = 0; i < nodes; i++)
    {
        for (int j = i + 1; j < nodes; j++)
        {
            if (huffcodes[i].freq < huffcodes[j].freq)
            {
                temphuff = huffcodes[i];
                huffcodes[i] = huffcodes[j];
                huffcodes[j] = temphuff;
            }
        }
    }

    float sumprob;
    int sumpix;
    int n = 0;
    int nextnode = nodes;

    while (n < nodes - 1)
    {
        sumprob = huffcodes[nodes - n - 1].freq + huffcodes[nodes - n - 2].freq;
        sumpix = huffcodes[nodes - n - 1].pix + huffcodes[nodes - n - 2].pix;

        pix_freq[nextnode].pix = sumpix;
        pix_freq[nextnode].freq = sumprob;
        pix_freq[nextnode].left = &pix_freq[huffcodes[nodes - n - 2].arrloc];
        pix_freq[nextnode].right = &pix_freq[huffcodes[nodes - n - 1].arrloc];
        *(pix_freq[nextnode].code) = '\0';

        int i = 0;

        while (sumprob <= huffcodes[i].freq)
            i++;

        for (int k = nodes - 1; k >= 0; k--)
        {
            if (k == i)
            {
                huffcodes[k].pix = sumpix;
                huffcodes[k].freq = sumprob;
                huffcodes[k].arrloc = nextnode;
            }
            else if (k > i)
                huffcodes[k] = huffcodes[k - 1];
        }

        n++;
        nextnode++;
    }

    char left = '0';
    char right = '1';

    for (int i = totalnodes - 1; i >= nodes; i--)
    {
        if (pix_freq[i].left != NULL)
            strconcat(pix_freq[i].left->code, pix_freq[i].code, left);
        if (pix_freq[i].right != NULL)
            strconcat(pix_freq[i].right->code, pix_freq[i].code, right);
    }

    printf("\n*** Huffman Codes ***\n");
    printf("Pixel Values ->         Code\n");
    for (int i = 0; i < nodes; i++)
        printf("     %-3d     ->   %s\n", pix_freq[i].pix, pix_freq[i].code);

    int new_bits = 0, original_bits = height * width * bpp;

    ftemp = 0;
    for (int i = 0; i < nodes; i++)
        ftemp += pix_freq[i].freq * strlen(pix_freq[i].code);

    for(int i = 0, j = 0; i < 256; i++)
        if(hist[i] != 0)
        {
            new_bits += hist[i] * strlen(pix_freq[j].code);
            j++;
        }

    printf("\n*** Compression Description ***\n");
    printf("- Original image had %d bits\n", original_bits);
    printf("- Compressed image has %d bits\n", new_bits);
    printf("- Average number of bits per pixel: %f\n", ftemp);

    return 0;
}
