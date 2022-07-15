# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <stdbool.h>

void Pause_program(char const *prompt);

int main(int argc, char const *argv[])
{
    int const num_useless_lines = 4u;
    bool is_load_inp_ori = false;
    bool is_load_std_ori = false;
    bool is_terminated_normally = false;
    char if_name[BUFSIZ + 1] = "";
    char of_inp_name[BUFSIZ + 1] = "";
    char of_std_name[BUFSIZ + 1] = "";
    char buf[BUFSIZ + 1] = "";
    char *chr_pos = NULL;
    char const coord_locator_inp[] = "Input orientation:";
    char const coord_locator_std[] = "Standard orientation:";
    char const station_point_found[] = "Stationary point found.";
    char const num_atoms_locator[] = "NAtoms=";
    char const normal_termination[] = "Normal termination of Gaussian";
    FILE *if_file = NULL;
    FILE *of_inp_file = NULL;
    FILE *of_std_file = NULL;
    int frame = 0;
    int num_atoms = 0;
    int index = 0;
    int cenNum = 0;
    int atomic_number = 0;
    int atomType = 0;
    double coord_x = 0.;
    double coord_y = 0.;
    double coord_z = 0.;
    char const *elements[] = {"", 
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
         "Db", "Sg", "Bh", "Hs", "Mt", "Ds", "Rg", "Cn", 
         "Nh", "Fl", "Mc", "Lv", "Ts", "Og"};
    int const max_atomic_number = sizeof(elements) / sizeof(char const *) - 1;
    size_t read_pos = 0ll;
    char ene_type_str[11] = "";
    double ene = 0.0;
    char *tok = NULL;

    if (argc - 1 == 0)
    {
        printf("Convert optimization output of Gaussian to xyz trajectory file.\n");
    }

    /*  Get file name */
    if (argc - 1 >= 2)
    {
        fprintf(stderr, "Error! At most 1 argument is required, but got %d.\n", argc - 1);
        exit(EXIT_FAILURE);
    }
    else if (argc - 1 == 1)
    {
        if (! strcmp(argv[1], "--help") || ! strcmp(argv[1], "-h") || ! strcmp(argv[1], "/?"))
        {
            printf("Usage: %s [GAUSSIAN_OPTIMIZATION_OUT_FILE]\n", argv[0]);
            printf("This program seperate coordinates from an output file of a Gaussian optimization task \n");
            printf("into multi-frame xyz files. The input orientation and the standard orientation will be \n");
            printf("saved individually.\n");
            Pause_program("Press <Enter> to exit ...");
            exit(EXIT_SUCCESS);
        }
        strcpy(if_name, argv[1]);
    }
    else
    {
        printf("Input file name, e.g. Desktop\\test.out\n");
        while (! fgets(buf, BUFSIZ, stdin))
        {
            ;
        }
        if (buf[strlen(buf) - 1] == '\n')
        {
            buf[strlen(buf) - 1] = '\0';
        }
        if (buf[0] == '\'' || buf[0] == '\"')
        {
            buf[strlen(buf) - 1] = '\0';
            buf[0] = '\0';
            strcpy(if_name, buf + 1);
        }
        else
        {
            strcpy(if_name, buf);
        }
    }

    /*  Check filename extension  */
    /*  Please note this short-circuit evalutation trick and think twice.  */
    // if (strlen(if_name) <= strlen(".xyz") || strcmp(if_name + strlen(if_name) - strlen(".out"), ".out") && \
    //     strcmp(if_name + strlen(if_name) - strlen(".log"), ".log"))
    if (! strrchr(if_name, '.') || strcmp(strrchr(if_name, '.'), ".out") && strcmp(strrchr(if_name, '.'), ".log"))
    {
        fprintf(stderr, "Error! The suffix of the input name should be either \".out\" or \".log\".\n");
        exit(EXIT_FAILURE);
    }
    strcpy(of_inp_name, if_name);
    strcpy(of_std_name, if_name);
    strcpy(of_inp_name + strlen(if_name) - strlen(".out"), "_inp_ori.xyz");
    strcpy(of_std_name + strlen(if_name) - strlen(".out"), "_std_ori.xyz");

    /*  Open files  */
    if_file = fopen(if_name, "rt");
    if (! if_file)
    {
        fprintf(stderr, "Error! Cannot open%s! Check your path and file name.\n", if_name);
        Pause_program("");
        exit(EXIT_FAILURE);
    }

    /*  Get atom amount, and test if there is standard orientation  */
    num_atoms = 0;
    while (fgets(buf, BUFSIZ, if_file))
    {
        if (chr_pos = strstr(buf, num_atoms_locator))
        {
            sscanf(chr_pos + strlen(num_atoms_locator), "%d", & num_atoms);
            break;
        }
    }
    if (! num_atoms)
    {
        fprintf(stderr, "Error! Cannot get amount of atoms through \"%s\".\n", num_atoms_locator);
        exit(EXIT_FAILURE);
    }
    if (argc == 1)
    {
        printf("The number of atoms: %d\n", num_atoms);
    }
    rewind(if_file);

    /* obtain energy type */
    while (true)
    {
        if (! fgets(buf, BUFSIZ, if_file))
        {
            fprintf(stderr, "Error! Cannot determine energy type.\n");
            fclose(if_file);
            if_file = NULL;
            exit(EXIT_FAILURE);
        }
        if (strstr(buf, "Energy="))
        {
            strcpy(ene_type_str, "MM");
            break;
        }
        if (strstr(buf, "SCF Done"))
        {
            break;
        }
    }
    if (strcmp(ene_type_str, "MM")) /* found "SCF Done" */
    {
        while (true)
        {
            if (! fgets(buf, BUFSIZ, if_file) || strstr(buf, "Population analysis"))
            {
                strcpy(ene_type_str, "SCF");
                break;
            }
            if (strstr(buf, "EUMP2 ="))
            {
                strcpy(ene_type_str, "MP2");
                break;
            }
            if (! strncmp(buf, " E2(", strlen(" E2(")))
            {
                strcpy(ene_type_str, "DFTPT2");
                break;
            }
            if (strstr(buf, "E(CIS/TDA)"))
            {
                strcpy(ene_type_str, "CIS/TDA");
                break;
            }
            if (strstr(buf, "E(TD-HF/TD-DFT)"))
            {
                strcpy(ene_type_str, "TD");
                break;
            }
        }
    }
    rewind(if_file);

    /*  Check whether input orientation or standard orientation exists  */
    while (fgets(buf, BUFSIZ, if_file))
    {
        if (strstr(buf, coord_locator_inp))
        {
            is_load_inp_ori = true;
            break;
        }
    }
    rewind(if_file);
    while (fgets(buf, BUFSIZ, if_file))
    {
        if (strstr(buf, coord_locator_std))
        {
            is_load_std_ori = true;
            break;
        }
    }
    rewind(if_file);
    if (! is_load_inp_ori && ! is_load_std_ori)
    {
        fprintf(stderr, "Error! Neither input orientation nor standart orientation could be find.\n");
        exit(EXIT_FAILURE);
    }

    /*  Open files for output  */
    if (is_load_inp_ori)
    {
        of_inp_file = fopen(of_inp_name, "rt");
        if (of_inp_file)
        {
            fprintf(stderr, "Error! File \"%s\" already exists, please remove or rename it first.\n", of_inp_name);
            exit(EXIT_FAILURE);
        }
        of_inp_file = fopen(of_inp_name, "wt");
        if (! of_inp_file)
        {
            fprintf(stderr, "Error! Cannot open \"%s\" for writing! Check your path and file name.\n", of_inp_name);
            Pause_program("");
            exit(EXIT_FAILURE);
        }
    }
    if (is_load_std_ori)
    {
        of_std_file = fopen(of_std_name, "rt");
        if (of_std_file)
        {
            fprintf(stderr, "Error! File \"%s\" already exists, please remove or rename it first.\n", of_std_name);
            exit(EXIT_FAILURE);
        }
        of_std_file = fopen(of_std_name, "wt");
        if (! of_std_file)
        {
            fprintf(stderr, "Error! Cannot open \"%s\" for writing! Check your path and file name.\n", of_std_name);
            Pause_program("");
            exit(EXIT_FAILURE);
        }
    }

    /*  Get coordinate  */
    if (is_load_inp_ori)
    {
        frame = 0u;
        while (fgets(buf, BUFSIZ, if_file))
        {
            if (strstr(buf, station_point_found))
            {
                break;
            }
            if (strstr(buf, coord_locator_inp))
            {
                ++ frame;
                read_pos = ftell(if_file);
                /* for energy */
                if (strstr(ene_type_str, "MM"))
                {
                    while (fgets(buf, BUFSIZ, if_file))
                    {
                        if (tok = strstr(buf, "Energy="))
                        {
                            break;
                        }
                    }
                    tok += strlen("Energy=");
                    sscanf(tok, "%lg", & ene);
                }
                else if (strstr(ene_type_str, "SCF"))
                {
                    while (fgets(buf, BUFSIZ, if_file))
                    {
                        if (strstr(buf, "SCF Done"))
                        {
                            break;
                        }
                    }
                    tok = strchr(buf, '=') + strlen("=");
                    sscanf(tok, "%lg", & ene);
                }
                else if (strstr(ene_type_str, "MP2"))
                {
                    while (fgets(buf, BUFSIZ, if_file))
                    {
                        if (tok = strstr(buf, "EUMP2"))
                        {
                            break;
                        }
                    }
                    tok = strchr(tok, ' ') + strlen("=");
                    * strchr(tok, 'D') = 'E';
                    sscanf(tok, "%lg", & ene);
                }
                else if (strstr(ene_type_str, "DFTPT2"))
                {
                    while (fgets(buf, BUFSIZ, if_file))
                    {
                        if (! strncmp(buf, " E2(", strlen(" E2(")))
                        {
                            break;
                        }
                    }
                    tok = strstr(buf, "E(");
                    tok = strchr(tok, '=') + strlen("=");
                    * strchr(tok, 'D') = 'E';
                    sscanf(tok, "%lg", & ene);
                }
                else if (strstr(ene_type_str, "CIS/TDA"))
                {
                    while (fgets(buf, BUFSIZ, if_file))
                    {
                        if (tok = strstr(buf, "E(CIS/TDA)"))
                        {
                            break;
                        }
                    }
                    tok = strchr(buf, '=') + strlen("=");
                    sscanf(tok, "%lg", & ene);
                }
                else if (strstr(ene_type_str, "TD"))
                {
                    while (fgets(buf, BUFSIZ, if_file))
                    {
                        if (tok = strstr(buf, "E(TD-HF/TD-DFT)"))
                        {
                            break;
                        }
                    }
                    tok = strchr(buf, '=') + strlen("=");
                    sscanf(tok, "%lg", & ene);
                }
                else
                {
                    /* should never happen */
                    ;
                }
                tok = NULL;
                fseek(if_file, read_pos, SEEK_SET);
                /* for coordinates */
                for (index = 0; index < num_useless_lines; ++ index)
                {
                    fgets(buf, BUFSIZ, if_file);
                }
                fprintf(of_inp_file, "%d\n", num_atoms);
                fprintf(of_inp_file, "frame %d: energy = %17.10lf Hartree (energy type: \"%s\")\n", frame, ene, ene_type_str);
                for (index = 1; index <= num_atoms; ++ index)
                {
                    fgets(buf, BUFSIZ, if_file);
                    if (sscanf(buf, "%*d %d %*d %lg %lg %lg", & atomic_number, & coord_x, & coord_y, & coord_z) != 4)
                    {
                        fprintf(stderr, "Error! Cannot get information of the atom with index %d in frame %d.\n", index, frame);
                        exit(EXIT_FAILURE);
                    }
                    if (atomic_number <= max_atomic_number)
                    {
                        fprintf(of_inp_file, "%-2s%8s%12.8f%8s%12.8f%8s%12.8f\n", elements[atomic_number], "", coord_x, "", coord_y, "", coord_z);
                    }
                    else
                    {
                        fprintf(stderr, "Warning! Atomic number larger overflowed in current periodic table, using \"Bq\" as atomic symbol.\n");
                        fprintf(of_inp_file, "%-2s%8s%12.8f%8s%12.8f%8s%12.8f\n", "Bq", "", coord_x, "", coord_y, "", coord_z);
                    }

                }
            }
        }
    }
    rewind(if_file);
    frame = 0u;
    if (is_load_std_ori)
    {
        frame = 0u;
        while (fgets(buf, BUFSIZ, if_file))
        {
            if (strstr(buf, station_point_found))
            {
                break;
            }
            if (strstr(buf, coord_locator_std))
            {
                ++ frame;
                read_pos = ftell(if_file);
                /* for energy */
                if (strstr(ene_type_str, "MM"))
                {
                    while (fgets(buf, BUFSIZ, if_file))
                    {
                        if (tok = strstr(buf, "Energy="))
                        {
                            break;
                        }
                    }
                    tok += strlen("Energy=");
                    sscanf(tok, "%lg", & ene);
                }
                else if (strstr(ene_type_str, "SCF"))
                {
                    while (fgets(buf, BUFSIZ, if_file))
                    {
                        if (strstr(buf, "SCF Done"))
                        {
                            break;
                        }
                    }
                    tok = strchr(buf, '=') + strlen("=");
                    sscanf(tok, "%lg", & ene);
                }
                else if (strstr(ene_type_str, "MP2"))
                {
                    while (fgets(buf, BUFSIZ, if_file))
                    {
                        if (tok = strstr(buf, "EUMP2"))
                        {
                            break;
                        }
                    }
                    tok = strchr(tok, ' ') + strlen("=");
                    * strchr(tok, 'D') = 'E';
                    sscanf(tok, "%lg", & ene);
                }
                else if (strstr(ene_type_str, "DFTPT2"))
                {
                    while (fgets(buf, BUFSIZ, if_file))
                    {
                        if (! strncmp(buf, " E2(", strlen(" E2(")))
                        {
                            break;
                        }
                    }
                    tok = strstr(buf, "E(");
                    tok = strchr(tok, '=') + strlen("=");
                    * strchr(tok, 'D') = 'E';
                    sscanf(tok, "%lg", & ene);
                }
                else if (strstr(ene_type_str, "CIS/TDA"))
                {
                    while (fgets(buf, BUFSIZ, if_file))
                    {
                        if (tok = strstr(buf, "E(CIS/TDA)"))
                        {
                            break;
                        }
                    }
                    tok = strchr(buf, '=') + strlen("=");
                    sscanf(tok, "%lg", & ene);
                }
                else if (strstr(ene_type_str, "TD"))
                {
                    while (fgets(buf, BUFSIZ, if_file))
                    {
                        if (tok = strstr(buf, "E(TD-HF/TD-DFT)"))
                        {
                            break;
                        }
                    }
                    tok = strchr(buf, '=') + strlen("=");
                    sscanf(tok, "%lg", & ene);
                }
                else
                {
                    /* should never happen */
                    ;
                }
                tok = NULL;
                fseek(if_file, read_pos, SEEK_SET);
                /* for coordinates */
                for (index = 0; index < num_useless_lines; ++ index)
                {
                    fgets(buf, BUFSIZ, if_file);
                }
                fprintf(of_std_file, "%d\n", num_atoms);
                fprintf(of_std_file, "frame %d: energy = %17.10lf Hartree (energy type: \"%s\")\n", frame, ene, ene_type_str);
                for (index = 1; index <= num_atoms; ++ index)
                {
                    fgets(buf, BUFSIZ, if_file);
                    if (sscanf(buf, "%*d %d %*d %lg %lg %lg", & atomic_number, & coord_x, & coord_y, & coord_z) != 4)
                    {
                        fprintf(stderr, "Error! Cannot get information of the atom with index %d in frame %d.\n", index, frame);
                        exit(EXIT_FAILURE);
                    }
                    if (atomic_number <= max_atomic_number)
                    {
                        fprintf(of_std_file, "%-2s%8s%12.8f%8s%12.8f%8s%12.8f\n", elements[atomic_number], "", coord_x, "", coord_y, "", coord_z);
                    }
                    else
                    {
                        fprintf(stderr, "Warning! Atomic number larger overflowed in current periodic table, using \"Bq\" as atomic symbol.\n");
                        fprintf(of_std_file, "%-2s%8s%12.8f%8s%12.8f%8s%12.8f\n", "Bq", "", coord_x, "", coord_y, "", coord_z);
                    }

                }
            }
        }
    }

    /*  close files  */
    if (is_load_inp_ori)
    {
        fclose(of_inp_file);
        of_inp_file = NULL;
    }
    if (is_load_std_ori)
    {
        fclose(of_std_file);
        of_std_file = NULL;
    }

    if (argc == 1 && is_load_inp_ori)
    {
        printf("The trajectory as input orientation has been saved to %s\n", of_inp_name);
    }
    if (argc == 1 && is_load_std_ori)
    {
        printf("The trajectory as standard orientation has been saved to %s\n", of_std_name);
    }
    while (fgets(buf, BUFSIZ, if_file))
    {
        if (strstr(buf, normal_termination))
        {
            is_terminated_normally = true;
            break;
        }
    }
    if (! is_terminated_normally)
    {
        fprintf(stderr, "Warning: Gaussian job may still be running or did not terminate normally.\n");
    }
    fclose(if_file);
    if_file = NULL;
    if (argc == 1)
    {
        printf("Done!\n");
    }

    /*  Pause  */
    if (argc == 1)
    {
        Pause_program("Press <Enter> to continue...");
    }

    return 0;
}

void Pause_program(char const *prompt)
{
    char pauser = '\0';

    if (prompt)
    {
        printf("%s\n", prompt);
    }
    while ((pauser = getchar()) != '\n' && pauser != EOF)
    {
        ;
    }

    return;
}

