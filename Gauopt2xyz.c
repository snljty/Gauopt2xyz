# include <stdio.h>
# include <stdlib.h>
# include <string.h>

void PauseProgram(const char * prompt);

int main(int argc, char *argv[])
{
  # define ERROR_OPEN_FILE 1
  # define UselessLines 4
  typedef enum {False, True} Bool;
  Bool isStationPoint = False;
  Bool isLoadStdOri = False;
  char if_name[BUFSIZ] = "";
  char of_inp_name[BUFSIZ] = "";
  char of_std_name[BUFSIZ] = "";
  char line[BUFSIZ] = "";
  const char coorLocatorInp[] = "                          Input orientation:                          \n";
  const char coorLocatorStd[] = "                         Standard orientation:                         \n";
  const char paraBreaker[] = " ---------------------------------------------------------------------\n";
  const char statPointFound[] = "    -- Stationary point found.\n";
  char * tempChar = NULL;
  FILE * if_file = NULL;
  FILE * of_inp_file = NULL;
  FILE * of_std_file = NULL;
  unsigned int frame = 0;
  unsigned int numAtoms = 0;
  unsigned int index = 0;
  char buf[BUFSIZ] = "";
  unsigned int cenNum = 0;
  unsigned int atomNum = 0;
  unsigned int atomType = 0;
  float xCoor = 0.;
  float yCoor = 0.;
  float zCoor = 0.;
  char c = '\0';
  const char * AtomName[] = {"", 
     "H" , "He", "Li", "Be", "B" , "C" , "N" , "O" ,
     "F" , "Ne", "Na", "Mg", "Al", "Si", "P" , "S" ,
     "Cl", "Ar", "K" , "Ca", "Sc", "Ti", "V" , "Cr",
     "Mn", "Fe", "Co", "Ni", "Cu", "Zn", "Ga", "Ge",
     "As", "Se", "Br", "Kr", "Rb", "Sr", "Y" , "Zr",
     "Nb", "Mo", "Tc", "Ru", "Rh", "Pd", "Ag", "Cd",
     "In", "Sn", "Sb", "Te", "I" , "Xe", "Cs", "Ba",
     "La", "Ce", "Pr", "Nd", "Pm", "Sm", "Eu", "Gd",
     "Tb", "Dy", "Ho", "Er", "Tm", "Yb", "Lu", "Hf",
     "Ta", "W" , "Re", "Os", "Ir", "Pt", "Au", "Hg",
     "Tl", "Pb", "Bi", "Po", "At", "Rn", "Fr", "Ra",
     "Ac", "Th", "Pa", "U" , "Np", "Pu", "Am", "Cm",
     "Bk", "Cf", "Es", "Fm", "Md", "No", "Lr", "Rf",
     "Db", "Sg", "Bh", "Hs", "Mt", "Ds", "Rg", "Cn"};

  /*  pre work  */
  puts(" Convert optimization output of Gaussian to xyz trajectory file");


  /*  Get file name */
  if(argc > 1)
    strcpy(if_name, argv[1]);
  else
  {
    puts(" Input file name, e.g. Desktop\\test.out");
    scanf("%[^\n]%c", if_name, & c);
  }
  if(if_name[0] == '\'' || if_name[0] == '\"')
  {
    for(index = 0; index < strlen(if_name) - 2 * strlen("\""); index ++)
      if_name[index] = if_name[index + 1];
    if_name[index] = '\0';
  }
  strncpy(of_inp_name, if_name, strlen(if_name) - strlen(".out"));
  strcpy(of_std_name, of_inp_name);
  strcat(of_inp_name, "_inp_ori.xyz");
  strcat(of_std_name, "_std_ori.xyz");

  /*  Open files  */
  if_file = fopen(if_name, "r");
  if(! if_file)
  {
    printf("Error! cannot open%s! Check your path and file name.\n", if_name);
    PauseProgram("");
    exit(ERROR_OPEN_FILE);
  }
  of_inp_file = fopen(of_inp_name, "w");
  if(! of_inp_file)
  {
    printf("Error! cannot open%s! Check your path and file name.\n", of_inp_name);
    PauseProgram("");
    exit(ERROR_OPEN_FILE);
  }

  /*  Get atom amount, and test if there is standard orientation  */
  numAtoms = 0;
  while(True)
  {
    fgets(line, BUFSIZ, if_file);
    if(feof(if_file))
      break;
    if(! strcmp(line, coorLocatorInp))
    {
      if(numAtoms > 0)
        break;
      for(index = 0; index < UselessLines; index ++)
        fgets(line, BUFSIZ, if_file);
      while(True)
      {
        fgets(line, BUFSIZ, if_file);
        if(! strcmp(line, paraBreaker))
          break;
        numAtoms += 1;
      }
    }
    if(! strcmp(line, coorLocatorStd))
    {
      isLoadStdOri = True;
      of_std_file = fopen(of_std_name, "w");
      if(! of_std_file)
      {
        printf("Error! cannot open%s! Check your path and file name.\n", of_std_name);
        PauseProgram("");
        exit(ERROR_OPEN_FILE);
      }
      break;
    }
  }

  /*  Get coordinate  */
  rewind(if_file);
  while(True)
  {
    fgets(line, BUFSIZ, if_file);
    if(feof(if_file))
      break;
    if(! strcmp(line, statPointFound))
      isStationPoint = True;
    if(! strcmp(line, coorLocatorInp))
    {
      if(isStationPoint)
        break;
      for(index = 0; index < UselessLines; index ++)
        fgets(line, BUFSIZ, if_file);
      frame ++;
      fprintf(of_inp_file, "      %6u\n", numAtoms);
      fprintf(of_inp_file, "Frame%5u\n", frame);
      for(index = 0; index < numAtoms; index ++)
      {
        fgets(line, BUFSIZ, if_file);
        sscanf(line, " %6u        %3u          %2u      %10f  %10f  %10f",
               & cenNum, & atomNum, & atomType, & xCoor, & yCoor, & zCoor);
        fprintf(of_inp_file, "%-2s    %12.8f    %12.8f    %12.8f\n", AtomName[atomNum], xCoor, yCoor, zCoor);
      }
    }
    if(isLoadStdOri)
    {
      if(! strcmp(line, coorLocatorStd))
      {
        for(index = 0; index < UselessLines; index ++)
          fgets(line, BUFSIZ, if_file);
        fprintf(of_std_file, "      %6u\n", numAtoms);
        fprintf(of_std_file, "Frame%5u\n", frame);
        for(index = 0; index < numAtoms; index ++)
        {
          fgets(line, BUFSIZ, if_file);
          sscanf(line, " %6u        %3u          %2u      %10f  %10f  %10f",
                 & cenNum, & atomNum, & atomType, & xCoor, & yCoor, & zCoor);
          fprintf(of_std_file, "%-2s    %12.8f    %12.8f    %12.8f\n", AtomName[atomNum], xCoor, yCoor, zCoor);
        }
      }
    }
  }
  while(True)
  {
    fgets(line, BUFSIZ, if_file);
    if(feof(if_file))
      break;
  }
  putc('\n', of_inp_file);
  putc('\n', of_std_file);

  /*  Close files  */
  fclose(if_file);
  if_file = NULL;
  fclose(of_inp_file);
  of_inp_file = NULL;
  fclose(of_std_file);
  of_std_file = NULL;

  /*  suf work  */
  printf(" The number of atoms:    %6u\n", numAtoms);
  printf(" The trajectory as input orientation has been saved to %s\n", of_inp_name);
  if(isLoadStdOri)
    printf(" The trajectory as standard orientation has been saved to %s\n", of_std_name);
  if(! strstr(line, "Normal termination of Gaussian"))
    puts("Warning: Gaussian job may still be running or did not terminate normally");
  puts(" Done!");

  /*  Pause  */
  PauseProgram("Press Enter to continue...");

  return 0;
}

void PauseProgram(const char * prompt)
{
  char pauser = '\0';
  puts(prompt);
  while((pauser = getchar()) != '\n' && pauser != EOF)
    ;
}

