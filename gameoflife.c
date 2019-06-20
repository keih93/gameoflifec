#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <omp.h>

//OPTIONAL: comment this out for console output
//#define CONSOLE_OUTPUT

#define calcIndex(width, x,y)  ((y)*(width) + (x))
#define ALIVE 1
#define DEAD 0



#define START_TIMEMEASUREMENT(name)   struct timeval __FILE__##__func__##name##actualtime; \
gettimeofday(&__FILE__##__func__##name##actualtime, NULL); \
double __FILE__##__func__##name##s_time = (double)__FILE__##__func__##name##actualtime.tv_sec+((double)__FILE__##__func__##name##actualtime.tv_usec/1000000.0)


#define END_TIMEMEASUREMENT(name, res) gettimeofday(&__FILE__##__func__##name##actualtime, NULL); \
res = (double)__FILE__##__func__##name##actualtime.tv_sec+((double)__FILE__##__func__##name##actualtime.tv_usec/1000000.0) -__FILE__##__func__##name##s_time




void myexit (const char * s, ...) {
  va_list args;
  va_start(args, s);
  vprintf (s, args);
  printf ("\n");
  va_end(args);
  abort ();
}

char vtk_header[2048];
void create_vtk_header (char * header, int width, int height, int timestep) {
  char buffer[1024];
  header[0] = '\0';
  strcat (header, "# vtk DataFile Version 3.0\n");
  snprintf (buffer, sizeof(buffer), "Gameoflife timestep %d \n", timestep);
  strcat (header, buffer);
  strcat (header, "BINARY\n");
  strcat (header, "DATASET STRUCTURED_POINTS\n");
  snprintf (buffer, sizeof(buffer), "DIMENSIONS %d %d 1\n", width, height);
  strcat (header, buffer);
  strcat (header, "SPACING 1.0 1.0 1.0\n");
  strcat (header, "ORIGIN 0 0 0\n");
  snprintf (buffer, sizeof(buffer), "POINT_DATA %d\n", width * height);
  strcat (header, buffer);
  strcat (header, "SCALARS data char 1\n");
  strcat (header, "LOOKUP_TABLE default\n");
}

void write_vtk_data (FILE * f, char * data, int length) {
  if (fwrite (data, sizeof(char), length, f) != length) {
    myexit ("Could not write vtk-Data");
  }
}

void write_field (char* currentfield, int width, int height, int timestep) {
#ifdef CONSOLE_OUTPUT
  printf("\033[H");
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) printf(ALIVE == currentfield[calcIndex(width, x,y)] ? "\033[07m  \033[m" : "  ");
    printf("\033[E");
  }
  fflush(stdout);
  printf("\ntimestep=%d",timestep);
  usleep(80000);
#else
  if (timestep == 0) {
    mkdir("./gol/", 0777);
    create_vtk_header (vtk_header, width, height, timestep);
  }
  printf ("writing timestep %d\n", timestep);
  FILE *fp;       // The current file handle.
  char filename[1024];
  snprintf (filename, 1024, "./gol/gol-%05d.vtk", timestep);
  fp = fopen (filename, "w");
  write_vtk_data (fp, vtk_header, strlen (vtk_header));
  write_vtk_data (fp, currentfield, width * height);
  fclose (fp);
  printf ("finished writing timestep %d\n", timestep);
#endif
}

int countLifingsPeriodic(char* currentfield, int x, int y, int w, int h){
  int n = 0;
  for(int y1 = y - 1; y1 <= y+1; y1++){
    for (int x1 = x - 1; x1 <= x + 1; x1++){
      if(currentfield[calcIndex(w, (x1 +w ) % w, (y1 +h) %h)]){
        n++;
      }
    }
  }
  return n;
}

void evolve (char* currentfield, char* newfield, int width, int height, int widthsize, int heightsize) {
  // TODO traverse through each voxel and implement game of live logic
  // HINT: avoid boundaries
int num_threads = widthsize * heightsize;
omp_set_dynamic(0);
#pragma omp parallel num_threads(num_threads)
{
      int this_thread = omp_get_thread_num();
      int my_width_start = (this_thread % widthsize)*(width/widthsize);
      int my_width_end = ((this_thread % widthsize)+1)*(width/widthsize);
      int my_height_start = (this_thread / heightsize)*(width/heightsize);
      int my_height_end = ((this_thread / heightsize)+1)*(width/heightsize);
      int n = 0;
      for (int y = my_height_start; y < my_height_end; y++){
        for (int x = my_width_start; x < my_width_end; x++){
            n = countLifingsPeriodic(currentfield, x , y , width, height);
            if (currentfield[calcIndex(width, x, y)]) n--;
            newfield[calcIndex(width, x, y)] = (n == 3 || (n == 2 && currentfield[calcIndex(width,x,y)]));
    }
  }
}
}

void filling_random (char * currentfield, int width, int height) {
  int i;
  for (int y = 1; y < height - 1; y++) {
    for (int x = 1; x < width - 1; x++) {
      i = calcIndex(width, x, y);
      currentfield[i] = (rand () < RAND_MAX / 10) ? 1 : 0; ///< init domain randomly
    }
  }
}

void filling_runner (char * currentfield, int width, int height) {
  int offset_x = width/3;
  int offset_y = height/2;
  currentfield[calcIndex(width, offset_x+0, offset_y+1)] = ALIVE;
  currentfield[calcIndex(width, offset_x+1, offset_y+2)] = ALIVE;
  currentfield[calcIndex(width, offset_x+2, offset_y+0)] = ALIVE;
  currentfield[calcIndex(width, offset_x+2, offset_y+1)] = ALIVE;
  currentfield[calcIndex(width, offset_x+2, offset_y+2)] = ALIVE;
}

void apply_periodic_boundaries(char * field, int width, int height){
  //TODO: implement periodic boundary copies
  int i, j, k, l;
  for (int y = 0; y < height - 1; y++) {
      i = calcIndex(width, width - 1, y);
      j = calcIndex(width, 1, y);
      l = calcIndex(width, 0, y);
      k = calcIndex(width, width - 2, y);
      field[i] = field[j];
      field[l] = field[k];
  }
int a, b, c, d;
  for (int x = 1; x < width - 1; x++) {
    a = calcIndex(width, x, height - 1);
    b = calcIndex(width, x, 1);
    d = calcIndex(width, x, 0);
    c = calcIndex(width, x, height - 2);
    field[a] = field[b];
    field[d] = field[c];
  }
}

void game (int width, int height, int num_timesteps, int widthsize, int heightsize) {
  char *currentfield = calloc (width * height, sizeof(char));
  char *newfield = calloc (width * height, sizeof(char));

  // TODO 1: use your favorite filling
  //filling_random (currentfield, width, height);
  filling_runner (currentfield, width, height);

  int time = 0;
  write_field (currentfield, width, height, time);
  //TODO 4: implement periodic boundary condition
  apply_periodic_boundaries(currentfield,width,height);
  for (time = 1; time <= num_timesteps; time++) {
    // TODO 2: implement evolve function (see above)
    evolve (currentfield, newfield, width, height, widthsize, heightsize);
    write_field (newfield, width, height, time);
    //TODO 4: implement periodic boundary condition
    apply_periodic_boundaries(newfield,width,height);

    //TODO 3: implement SWAP of the fields
    char *temp = currentfield;
    currentfield = newfield;
    newfield = temp;
  }

  free (currentfield);
  free (newfield);
}

int main (int c, char **v) {
  int width = 0, height = 0, num_timesteps, widthsize, heightsize;
  if (c == 6) {
    width = atoi (v[1]) + 2; ///< read width + 2 boundary cells (low x, high x)
    height = atoi (v[2]) + 2; ///< read height + 2 boundary cells (low y, high y)
    num_timesteps = atoi (v[3]); ///< read timesteps
    widthsize = atoi(v[4]);
    heightsize = atoi(v[5]);
    if (width <= 0) {
      width = 32; ///< default width
    }
    if (height <= 0) {
      height = 32; ///< default height
    }

    double elapsed_time;
    START_TIMEMEASUREMENT(measure_game_time);

    game (width, height, num_timesteps, widthsize, heightsize);

    END_TIMEMEASUREMENT(measure_game_time, elapsed_time);
    printf("time elapsed: %lf sec\n",elapsed_time);
  }
  else {
   myexit("Too less arguments, example: ./gameoflife <x size> <y size> <number of timesteps> <width> <height>");
  }
}
