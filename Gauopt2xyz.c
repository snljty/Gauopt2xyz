# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <stdbool.h>

void PauseProgram(const char *prompt);

int main(int argc, char const *argv[])
{
  # define ERROR_OPEN_FILE 1
  # define UselessLines 4
  int iarg = 0;
  bool isStationPoint = false;
  bool isLoadStdOri = false;
  char *if_name = NULL;
  char of_inp_name[BUFSIZ + 1] = "";
  char of_std_name[BUFSIZ + 1] = "";
  char line[BUFSIZ] = "";
  const char coorLocatorInp[] = "                          Input orientation:                          \n";
  const char coorLocatorStd[] = "                         Standard orientation:                         \n";
  const char paraBreaker[] = " ---------------------------------------------------------------------\n";
  const char statPointFound[] = "    -- Stationary point found.\n";
  FILE *if_file = NULL;
  FILE *of_inp_file = NULL;
  FILE *of_std_file = NULL;
  unsigned int frame = 0;
  unsigned int numAtoms = 0;
  unsigned int index = 0;
  char buf[BUFSIZ + 1] = "";
  unsigned int cenNum = 0;
  unsigned int atomNum = 0;
  unsigned int atomType = 0;
  float xCoor = 0.;
  float yCoor = 0.;
  float zCoor = 0.;
  const char *AtomName[] = {"", 
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
  if (argc > 1)
  {
    for (iarg = 1; iarg < argc; iarg ++)
    {
      if (! strcmp(argv[iarg], "--help") || ! strcmp(argv[iarg], "-h") || ! strcmp(argv[iarg], "/?"))
      {
        printf("Usage: %s [GAUSSIAN_OPTIMIZATION_OUT_FILE]\n", argv[0]);
        puts("This program seperate coordinates from an output file of a Gaussian optimization task ");
        puts("into multi-frame xyz files. The input orientation and the standard orientation will be ");
        puts("saved individually.");
        PauseProgram("Press <Enter> to exit ...");
        return 0;
      }
    }
    strcpy(buf, argv[1]);
  }
  else
  {
    puts(" Input file name, e.g. Desktop\\test.out");
    fgets(buf, BUFSIZ, stdin);
    while (buf[strlen(buf) - 1] == '\n')
      buf[strlen(buf) - 1] = '\0';
  }

  if_name = buf;
  if (buf[0] == '\'' || buf[0] == '\"')
  {
    buf[strlen(buf) - 1] = '\0';
    buf[0] = '\0';
    ++ if_name;
  }
  if (strcmp(if_name + strlen(if_name) - strlen(".out"), ".out") && \
      strcmp(if_name + strlen(if_name) - strlen(".log"), ".log"))
  {
    puts("Error! The suffix of the input name should be either \".out\" or \".log\".");
    exit(ERROR_OPEN_FILE);
  }
  strncpy(of_inp_name, if_name, strlen(if_name) - strlen(".out"));
  strcpy(of_std_name, of_inp_name);
  strcat(of_inp_name, "_inp_ori.xyz");
  strcat(of_std_name, "_std_ori.xyz");

  /*  Open files  */
  if_file = fopen(if_name, "r");
  if (! if_file)
  {
    printf("Error! Cannot open%s! Check your path and file name.\n", if_name);
    PauseProgram("");
    exit(ERROR_OPEN_FILE);
  }
  of_inp_file = fopen(of_inp_name, "w");
  if (! of_inp_file)
  {
    printf("Error! Cannot open%s! Check your path and file name.\n", of_inp_name);
    PauseProgram("");
    exit(ERROR_OPEN_FILE);
  }

  /*  Get atom amount, and test if there is standard orientation  */
  numAtoms = 0;
  while (true)
  {
    fgets(line, BUFSIZ, if_file);
    if (feof(if_file))
      break;
    if (! strcmp(line, coorLocatorInp))
    {
      if (numAtoms > 0)
        break;
      for (index = 0; index < UselessLines; index ++)
        fgets(line, BUFSIZ, if_file);
      while (true)
      {
        fgets(line, BUFSIZ, if_file);
        if(! strcmp(line, paraBreaker))
          break;
        numAtoms += 1;
      }
    }
    if (! strcmp(line, coorLocatorStd))
    {
      isLoadStdOri = true;
      of_std_file = fopen(of_std_name, "w");
      if (! of_std_file)
      {
        printf("Error! Cannot open%s! Check your path and file name.\n", of_std_name);
        PauseProgram("");
        exit(ERROR_OPEN_FILE);
      }
      break;
    }
  }

  /*  Get coordinate  */
  rewind(if_file);
  while (true)
  {
    fgets(line, BUFSIZ, if_file);
    if (feof(if_file))
      break;
    if (! strcmp(line, statPointFound))
      isStationPoint = true;
    if (! strcmp(line, coorLocatorInp))
    {
      if (isStationPoint)
        break;
      for (index = 0; index < UselessLines; index ++)
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
    if (isLoadStdOri)
    {
      if (! strcmp(line, coorLocatorStd))
      {
        for (index = 0; index < UselessLines; index ++)
          fgets(line, BUFSIZ, if_file);
        fprintf(of_std_file, "      %6u\n", numAtoms);
        fprintf(of_std_file, "Frame%5u\n", frame);
        for (index = 0; index < numAtoms; index ++)
        {
          fgets(line, BUFSIZ, if_file);
          sscanf(line, " %6u        %3u          %2u      %10f  %10f  %10f",
                 & cenNum, & atomNum, & atomType, & xCoor, & yCoor, & zCoor);
          fprintf(of_std_file, "%-2s    %12.8f    %12.8f    %12.8f\n", AtomName[atomNum], xCoor, yCoor, zCoor);
        }
      }
    }
  }
  while (true)
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
  if (isLoadStdOri)
    printf(" The trajectory as standard orientation has been saved to %s\n", of_std_name);
  if (! strstr(line, "Normal termination of Gaussian"))
    puts("Warning: Gaussian job may still be running or did not terminate normally");
  puts(" Done!");

  /*  Pause  */
  PauseProgram("Press Enter to continue...");

  return 0;
}

void PauseProgram(const char *prompt)
{
  char pauser = '\0';

  if (prompt)
    puts(prompt);
  while ((pauser = getchar()) != '\n' && pauser != EOF)
    ;

  return;
}

