#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<string.h>


int find_min(int**,int,int);
int find_max(int**,int,int);

typedef struct _PGMImage {
	int row;
	int col;
	int max_gray;
	int **matrix;
} PGMImage;

int **allocate_memory(int row, int col){
	int **matrix;
	int i;

	matrix = (int **)malloc(sizeof(int*) * row);//Allocate rows
	
	if (matrix == NULL){
		perror("MEMORY ALLOCATION FAILED");
		exit(EXIT_FAILURE);
	}
	
	for (i = 0; i < row; ++i){
		matrix[i] = (int *)malloc(sizeof(int) * col); // Allocate cols
		if(matrix[i] == NULL){
			perror("MEMORY ALLOCATION FAILED");
			exit(EXIT_FAILURE);
		}
	}
	return matrix;
}

void deallocate_image_memory(int **matrix, int row){
	int i;

	for (i = 0; i < row; ++i){
		free(matrix[i]); // Deallocate every rows
	}
	free(matrix);
}

int **normalize_matrix(int **matrix, int row, int col){
    int i,j, diff;
    int **normalized_matrix;
    normalized_matrix = allocate_memory(row,col);
    
    int min = find_min(matrix,row,col);
    int max = find_max(matrix,row,col);
    diff = max - min;

    for (i = 0; i < row; i++){
        for (j = 0; j < col; j++){
            normalized_matrix[i][j] = (int)(matrix[i][j]-min/diff);
        }
    } 

    return normalized_matrix;
}

int find_min(int **matrix, int row, int col){
    int i,j,min=1000000;
    for (i = 0; i < row; i++){
        for (j = 0; j < row; j++){
            if (matrix[i][j] < min ){
                min = matrix[i][j];
            }
        } 
    }
    return min;
}

int find_max(int **matrix, int row, int col){
    int i, j, max=-1000000;
    for (i = 0; i < row; i++) {
        for (j = 0; j < col; j++){
            if (matrix[i][j] > max){
                max = matrix[i][j];
            }
        }
    }
    return max;
}
PGMImage* readPGM(const char *file_name,PGMImage *data){
	FILE *pgmFile;
	char version[3];
	int i,j;
	
	pgmFile = fopen(file_name,"rb");
	if (pgmFile == NULL){
		perror("CANT READ IMAGE");
		exit(EXIT_FAILURE);
	}
	fgets(version,sizeof(version),pgmFile);
	if(strcmp(version,"P5")){
		printf("wrong file type");
		exit(EXIT_FAILURE);
	}
	
	fscanf(pgmFile, "%d", &data->col);
	fscanf(pgmFile, "%d", &data->row);
	fscanf(pgmFile, "%d", &data->max_gray);
	fgetc(pgmFile);
	data->matrix = allocate_memory(data->row,data->col);
       	for (i = 0; i < data->row; ++i) {
      		for (j = 0; j < data->col; ++j) {
               		data->matrix[i][j] = fgetc(pgmFile);
       		}
      	}

    	fclose(pgmFile);
    	return data;
}
/*and for writing*/

void writePGM(const char *filename, const PGMImage *data)
{
    FILE *pgmFile;
    int i, j;

    pgmFile = fopen(filename, "wb");
    if (pgmFile == NULL) {
        perror("cannot open file to write");
        exit(EXIT_FAILURE);
    }

    fprintf(pgmFile, "P5 ");
    fprintf(pgmFile, "%d %d ", data->col, data->row);
    fprintf(pgmFile, "%d ", data->max_gray);

        for (i = 0; i < data->row; ++i) {
            for (j = 0; j < data->col; ++j) {
                fputc(data->matrix[i][j], pgmFile);
            }
        }

    fclose(pgmFile);
    deallocate_image_memory(data->matrix, data->row);
}

PGMImage *apply_convolution(PGMImage *original_image,int **filter,int filter_size,int division_factor){
	PGMImage *new_image;
	int i,j,k,l,sum;
	new_image = (PGMImage *)malloc(sizeof(PGMImage));
	new_image = original_image;

	div_t padding;
	padding = div(filter_size,2);
	int s = padding.quot;

    for (i = s;i<new_image->row-s-2;i++){
        for (j = s;j<new_image->col-s-2;j++){
			if(i == 0 || j == 0  || i >= new_image->row-s || j >= new_image->col-s){ // resmin koselerine dokunmadm iceriden basla
				new_image->matrix[i][j] = original_image->matrix[i][j];
			}else{
				sum = 0;
				for (k = 0;k < filter_size;k++){
					for (l = 0;l < filter_size;l++){
						sum += filter[k][l] * original_image->matrix[i-1+k][j-1+l];
					}	
				}
				new_image->matrix[i][j] = abs((int)sum/division_factor); // burada bolulu bir seyler veya normalizasyon olabilir.
				if ( new_image->matrix[i][j] < 0 ) { 
				    new_image->matrix[i][j] = 0;
				}
			}
        }
    }
    new_image->matrix = normalize_matrix(new_image->matrix,new_image->row,new_image->col);
	return new_image;
}
void sobelFiltering(PGMImage *img){

    int **newPixel;

    int sobel_x[3][3] = { { 1,   0,  -1},
                          { 2,   0,  -2},
                          { 1,   0,  -1}};
    int sobel_y[3][3] = { { 1,   2,   1},
                          { 0,   0,   0},
                          {-1,  -2,  -1}};
    int x,y,i,j;
    int valX=0,valY=0;

    newPixel = allocate_memory(img->row,img->col);
    for (x=0; x<img->row; x++)
        for (y=0; y<img->col; y++){
            newPixel[x][y] = 0;
        }

    for (i=1; i<img->row-1; i++)
        for (j=1; j<img->col-1; j++){
            valX=0; valY=0;
            for (x = -1; x <= 1; x++)
                 for (y = -1; y <= 1; y++){
                    valX = valX + img->matrix[i+x][j+y] * sobel_x[1+x][1+y];
                    valY = valY + img->matrix[i+x][j+y] * sobel_y[1+x][1+y];
                    }
            newPixel[i][j] = (int)( sqrt(valX*valX + valY*valY) );
            /*if (newPixel[i][j] < 0)
                newPixel[i][j] = 0;
            else if (newPixel[i][j] > 255)
                newPixel[i][j] = 255;*/
        }
    for (x=0; x<img->row; x++)
        for (y=0; y<img->col; y++){
            img->matrix[x][y] = newPixel[x][y];
        }
    
    newPixel = normalize_matrix(newPixel,img->row,img->col);
    writePGM("sonuc.sobel.pgm",img);
    printf("\n[*]End of Sobel.!");
}

void laplacianFiltering(PGMImage *img){

    int **newPixel;

    int laplacian[3][3] = { { 1,   1,  1},
                          { 1,  -8,  1},
                          { 1,   1,  1}};
    int x,y,i,j;
    int val;

    newPixel = allocate_memory(img->row,img->col);
    for (x=0; x<img->row; x++)
        for (y=0; y<img->col; y++){
            newPixel[x][y] = 0;
        }

    for (i=1; i<img->row-1; i++)
        for (j=1; j<img->col-1; j++){
            val = 0;
            for (x = -1; x <= 1; x++)
                 for (y = -1; y <= 1; y++){
                    val = val + img->matrix[i+x][j+y] * laplacian[1+x][1+y];
                    }
            newPixel[i][j] = val;
            if (newPixel[i][j] < 0)
                newPixel[i][j] = 0;
            else if (newPixel[i][j] > 255)
                newPixel[i][j] = 255;
        }
    for (x=0; x<img->row; x++)
        for (y=0; y<img->col; y++){
            img->matrix[x][y] = newPixel[x][y];
        }

    //newPixel = normalize_matrix(newPixel,img->row,img->col);
    writePGM("sonuc.laplace.pgm",img);
    printf("\n[*]End of Laplace.!");
}

int main(){
    char filename[100];
    printf("Enter filename : ");
    scanf("%s",&filename);
	
	PGMImage *resim = (PGMImage *)malloc(sizeof(PGMImage)); 
	resim = readPGM(filename,resim);
	
	int **filter;
	int filter_size,i,j,division_factor;

	//gauss -> sobel ====== DONE
	//laplacian

	printf("Filter Size : ");
	scanf("%d",&filter_size);
	filter = allocate_memory(filter_size,filter_size);
	
	printf("Enter Filter : \n");
	for (i = 0;i < filter_size;i++){
		for (j = 0;j < filter_size;j++){
			scanf("%d",&filter[i][j]);
		}
	}

	printf("Division Factor: ");
	scanf("%d",&division_factor);
	
	writePGM("gauss.result.pgm",apply_convolution(resim,filter,filter_size,division_factor));	
	sobelFiltering(readPGM("gauss.result.pgm",resim));
	laplacianFiltering(readPGM(filename,resim));
	return 0;
}
