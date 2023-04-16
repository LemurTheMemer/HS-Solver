#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

#define NUM_EXCLUDE 17
#define STR_SIZE 50
#define NUM_DECKS 50
#define NUM_LINEUPS 20000
#define FIELD_SIZE 200
#define CLASSNAME_SIZE 3
#define MATCHUPS_FILENAME "matchups.txt"
#define FIELD_FILENAME "field.txt"
#define OUTPUT_FILENAME "output.txt"

// Function to determine deck class based on its name
char determineClass(char *deckName) 
{
    char arg1[STR_SIZE], arg2[STR_SIZE];
    char result;
    strcpy_s(arg2, STR_SIZE, deckName);
    while (1) 
    {
        sscanf_s(arg2, "%*s %[^\n]", arg1, (unsigned)_countof(arg1));
        if (!strcmp(arg1, "Death Knight"))
        {
            result = 'A';
            break;
        }
        else if (!strcmp(arg1, "Demon Hunter"))
        {
            result = 'B';
            break;
        }
        else if (!strcmp(arg1, "Druid"))
        {
            result = 'C';
            break;
        }
        else if (!strcmp(arg1, "Hunter"))
        {
            result = 'D';
            break;
        }
        else if (!strcmp(arg1, "Mage"))
        {
            result = 'E';
            break;
        }
        else if (!strcmp(arg1, "Paladin"))
        {
            result = 'F';
            break;
        }
        else if (!strcmp(arg1, "Priest"))
        {
            result = 'G';
            break;
        }
        else if (!strcmp(arg1, "Rogue"))
        {
            result = 'H';
            break;
        }
        else if (!strcmp(arg1, "Shaman"))
        {
            result = 'I';
            break;
        }
        else if (!strcmp(arg1, "Warlock"))
        {
            result = 'J';
            break;
        }
        else if (!strcmp(arg1, "Warrior"))
        {
            result = 'K';
            break;
        }
        strcpy_s(arg2, STR_SIZE, arg1);
    }
    return result;
}

// Function to read and store HSReplay matchup data as a 2-D matrix of winrates, returns number of decks in input file
int readMatchups(const char *filename, float matrix[NUM_DECKS][NUM_DECKS], char index[NUM_DECKS][STR_SIZE]) 
{
    char arg1[STR_SIZE], arg2[STR_SIZE], tempArg[STR_SIZE];
    int currHeroIdx = -1;
    int currVillainIdx = 0;
    int i;

    FILE *fp;
    fopen_s(&fp, filename, "r");
    if (fp == NULL)
    {
        perror("error opening matchups file");
        exit(EXIT_FAILURE);
    }
    // Skip BOM (3 characters)
    for (i = 0; i < 3; i++)
    {
        fscanf_s(fp, "%*c");
    }
    while (!feof(fp))
    {
        // Scan for hero deck
        fscanf_s(fp, "%s ", arg1, (unsigned)_countof(arg1));
        // Check for "Not enough data"
        if (!strcmp(arg1, "Not")) 
        {
            // Using 50.0% as a placeholder for insufficient data
            if (currVillainIdx > currHeroIdx)
            {
                matrix[currHeroIdx][currVillainIdx] = 0.50;
                matrix[currVillainIdx][currHeroIdx] = 0.50;
            }
            currVillainIdx++;
            // Skip 2 lines, then continue
            for (i = 0; i < 2; i++)
            {
                fscanf_s(fp, "\n%*[^\n]");
            }
            fscanf_s(fp, "\n");
        }
        else 
        {
            // Check for mirror
            if (currHeroIdx == currVillainIdx)
            {
                // Skip 2 lines, then continue
                for (i = 0; i < 2; i++)
                {
                    fscanf_s(fp, "\n%*[^\n]");
                }
                // Check for "TwitchTwitch VODs" line
                fscanf_s(fp, "%s", arg2, (unsigned)_countof(arg2));
                if (!strcmp(arg2, "TwitchTwitch"))
                {
                    // Skip one more line
                    fscanf_s(fp, "\n%*[^\n]\n");
                }
                else
                {
                    fseek(fp, -1 * sizeof(char) * strlen(arg2), SEEK_CUR);
                }
                matrix[currHeroIdx][currVillainIdx] = 0.50;
                currVillainIdx++;
                continue;
            }
            // Add rest of line to arg1
            fscanf_s(fp, "%[^\n]", arg2, (unsigned)_countof(arg2));
            strcat_s(arg1, STR_SIZE, " ");
            strcat_s(arg1, STR_SIZE, arg2);
            // Check if last element in index is NOT already arg1; if it isn't, add it
            if (currHeroIdx == -1 || strcmp(arg1, index[currHeroIdx]))
            {
                currHeroIdx++;
                strcpy_s(index[currHeroIdx], STR_SIZE, arg1);
                // Add class identifier character after null character
                index[currHeroIdx][strlen(index[currHeroIdx]) + 1] = determineClass(index[currHeroIdx]);
                // Return villainidx to 0
                currVillainIdx = 0;
                fseek(fp, -1 * sizeof(char) * strlen(arg1), SEEK_CUR);
                continue;
            }

            // Skip 3 lines
            for (i = 0; i < 3; i++)
            {
                fscanf_s(fp, "\n%*[^\n]");
            }
            // Check for "Twitch VODs" line
            fscanf_s(fp, "%s", arg2, (unsigned)_countof(arg2));
            if (!strcmp(arg2, "Twitch"))
            {
                // Skip one more line
                fscanf_s(fp, "%*[^\n]");
                fscanf_s(fp, "%s", arg2, (unsigned)_countof(arg2));
            }
            // Get winrate into matchup matrix
            sscanf_s(arg2, "%f", &(matrix[currHeroIdx][currVillainIdx]));
            matrix[currHeroIdx][currVillainIdx] /= 100;
            matrix[currVillainIdx][currHeroIdx] = matrix[currHeroIdx][currVillainIdx];

            currVillainIdx++;
        }
    }

    if (fclose(fp) != 0) 
    {
        perror("error closing matchups file");
        exit(EXIT_FAILURE);
    }

    return currHeroIdx + 1;
}
// Function to read and store field data as a 2-D matrix of indices, returns number of lineups in input file
int readField(const char *filename, int fieldLineups[][FIELD_SIZE], int decksPerLU, char index[NUM_DECKS][STR_SIZE], int numDecks) 
{
    char arg1[STR_SIZE];
    int i, j, cnt = 0;
    bool exists;

    FILE *fp;
    fopen_s(&fp, filename, "r");
    if (fp == NULL)
    {
        perror("error opening field file");
        exit(EXIT_FAILURE);
    }
    while (!feof(fp))
    {
        for (i = 0; i < decksPerLU; i++)
        {
            fscanf_s(fp, "%[^\n]\n", arg1, (unsigned)_countof(arg1));
            // Find each deck's corresponding index
            exists = false;
            for (j = 0; j < numDecks; j++)
            {
                if (!strcmp(arg1, index[j]))
                {
                    // Add the index to the current lineup
                    fieldLineups[i][cnt] = j;
                    exists = true;
                    break;
                }
            }
            if (!exists)
            {
                perror(("%s: No such deck", arg1));
                exit(EXIT_FAILURE);
            }
        }
        cnt++;
    }

    if (fclose(fp) != 0) 
    {
        perror("error closing field file");
        exit(EXIT_FAILURE);
    }

    return cnt;
}



// Function to populate allLineups with all possible lineups, returns number of lineups generated
int makeLineups(int allLineups[][NUM_LINEUPS], int decksPerLU, char index[NUM_DECKS][STR_SIZE], int numDecks)
{
    int cnt = 0, i , j, k, l;
    // Branch depending on lineup size
    if (decksPerLU == 3) // Bo3
    {
        for (i = 0; i < numDecks - 2 - NUM_EXCLUDE; i++)
        {
            for (j = i + 1; j < numDecks - 1 - NUM_EXCLUDE; j++)
            {
                // Exclude duplicate classes
                if (index[j][strlen(index[j]) + 1] == index[i][strlen(index[i]) + 1])
                {
                    continue;
                }
                for (k = j + 1; k < numDecks - NUM_EXCLUDE; k++)
                {
                    // Exclude duplicate classes
                    if (index[k][strlen(index[k]) + 1] == index[i][strlen(index[i]) + 1] || index[k][strlen(index[k]) + 1] == index[j][strlen(index[j]) + 1])
                    {
                        continue;
                    }
                    allLineups[0][cnt] = i;
                    allLineups[1][cnt] = j;
                    allLineups[2][cnt] = k;
                    cnt++;
                }

            }
        }
    }
    else if (decksPerLU == 4) // Bo5
    {
        for (i = 0; i < numDecks - 3; i++)
        {
            for (j = i + 1; j < numDecks - 2; j++)
            {
                // Exclude duplicate classes
                if (index[j][strlen(index[j]) + 1] == index[i][strlen(index[i]) + 1])
                {
                    continue;
                }
                for (k = j + 1; k < numDecks - 1; k++)
                {
                    // Exclude duplicate classes
                    if (index[k][strlen(index[k]) + 1] == index[i][strlen(index[i]) + 1] || index[k][strlen(index[k]) + 1] == index[j][strlen(index[j]) + 1])
                    {
                        continue;
                    }
                    for (l = k + 1; l < numDecks; l++)
                    {
                        // Exclude duplicate classes
                        if (index[l][strlen(index[l]) + 1] == index[i][strlen(index[i]) + 1] || index[l][strlen(index[l]) + 1] == index[j][strlen(index[j]) + 1] || index[l][strlen(index[l]) + 1] == index[k][strlen(index[k]) + 1])
                        {
                            continue;
                        }
                        allLineups[0][cnt] = i;
                        allLineups[1][cnt] = j;
                        allLineups[2][cnt] = k;
                        allLineups[3][cnt] = l;
                        cnt++;
                    }
                }

            }
        }
    }
    return cnt;
}
// Functions to solve for EWR by reducing the matrix
float calcEWR2x1(float matrix[2][1])
{
    return matrix[0][0] * matrix[1][0];
}
float calcEWR1x2(float matrix[1][2])
{
    return 1 - (1 - matrix[0][0]) * (1 - matrix[0][1]);
}
float calcEWR3x1(float matrix[3][1])
{
    return matrix[0][0] * matrix[1][0] * matrix[2][0];
}
float calcEWR1x3(float matrix[1][3])
{
    return 1 - (1 - matrix[0][0]) * (1 - matrix[0][1]) * (1 - matrix[0][2]);
}
float calcEWR2x2(float matrix[2][2])
{
    return ((2 * matrix[0][0] * matrix[1][0]) + (2 * matrix[0][1] * matrix[1][1]) + (matrix[0][1] * matrix[1][0]) + (matrix[0][0] * matrix[1][1])
        - (matrix[0][0] * matrix[0][1] * matrix[1][0]) - (matrix[0][0] * matrix[0][1] * matrix[1][1]) - (matrix[0][0] * matrix[1][0] * matrix[1][1]) - (matrix[0][1] * matrix[1][0] * matrix[1][1])) / 2;
}
float calcEWR3x2(float matrix[3][2])
{
    return ((matrix[0][0] + matrix[0][1]) * calcEWR2x2(&(matrix[1])) + (1 - matrix[0][0]) * calcEWR3x1(matrix + 1) + (1 - matrix[0][1]) * calcEWR3x1(matrix)) / 2;
}
float calcEWR2x3(float matrix[2][3])
{
    return ((matrix[0][0] + matrix[0][1] + matrix[0][2]) * calcEWR1x3(&(matrix[1])) + (1 - matrix[0][0]) * calcEWR2x2(matrix + 1) + (1 - matrix[0][2]) * calcEWR2x2(matrix)
            + (1 - matrix[0][1]) * (calcEWR2x1(matrix) + calcEWR2x1(matrix + 2)) / 2) / 3;
}
float calcEWR3x3(float matrix[3][3])
{
    return ((matrix[0][0] + matrix[0][1] + matrix[0][2]) * calcEWR2x3(&(matrix[1])) + (1 - matrix[0][0]) * calcEWR3x2(matrix + 1) + (1 - matrix[0][2]) * calcEWR3x2(matrix)
            + (1 - matrix[0][1]) * (calcEWR3x1(matrix) + calcEWR3x1(matrix + 2)) / 2) / 3;
}

// Function using the above functions to calculate EWRs for all lineups in allLineups
void calcAllEWRs(float EWRs[], int numLineups, int allLineups[][NUM_LINEUPS], int decksPerLU, int fieldLineups[][FIELD_SIZE], int fieldSize, float matchupMatrix[NUM_DECKS][NUM_DECKS])
{
    int i, j, k, l, heroBan, villainBan;
    float wrSum, banSum, banSumMax;
    bool pastBan1, pastBan2;
    // Branch depending on lineup size
    if (decksPerLU == 3) // Bo3
    {
        float matrix[2][2];
        for (i = 0; i < numLineups; i++)
        {
            wrSum = 0;
            for (j = 0; j < fieldSize; j++)
            {
                heroBan = 0;
                villainBan = 0; 
                // Find best villain deck (hero ban)
                banSumMax = 0;
                for (k = 0; k < 3; k++)
                {
                    banSum = 0;
                    for (l = 0; l < 3; l++)
                    {
                        banSum += matchupMatrix[fieldLineups[k][j]][allLineups[l][i]];
                    }
                    if (banSum > banSumMax)
                    {
                        banSumMax = banSum;
                        heroBan = k;
                    }
                }
                // Find best hero deck (villain ban)
                banSumMax = 0;
                for (k = 0; k < 3; k++)
                {
                    banSum = 0;
                    for (l = 0; l < 3; l++)
                    {
                        banSum += matchupMatrix[allLineups[k][i]][fieldLineups[l][j]];
                    }
                    if (banSum > banSumMax)
                    {
                        banSumMax = banSum;
                        villainBan = k;
                    }
                }
                // Populate winrate matrix, excluding banned decks
                pastBan1 = false;
                for (k = 0; k < 3; k++)
                {
                    if (k == villainBan)
                    {
                        pastBan1 = true;
                        continue;
                    }
                    pastBan2 = false;
                    for (l = 0; l < 3; l++)
                    {
                        if (l == heroBan)
                        {
                            pastBan2 = true;
                            continue;
                        }
                        matrix[k - pastBan1][l - pastBan2] = matchupMatrix[allLineups[k][i]][fieldLineups[l][j]];
                    }
                }
                // Calculate EWR for matchup, add to sum
                wrSum += calcEWR2x2(matrix);
            }
            // Populate result array
            EWRs[i] = wrSum / fieldSize;
        }
    }
    else if (decksPerLU == 4) // Bo5
    {
        float matrix[3][3];
        for (i = 0; i < numLineups; i++)
        {
            wrSum = 0;
            for (j = 0; j < fieldSize; j++)
            {
                heroBan = 0;
                villainBan = 0; 
                // Find best villain deck (hero ban)
                banSumMax = 0;
                for (k = 0; k < 4; k++)
                {
                    banSum = 0;
                    for (l = 0; l < 4; l++)
                    {
                        banSum += matchupMatrix[fieldLineups[k][j]][allLineups[l][i]];
                    }
                    if (banSum > banSumMax)
                    {
                        banSumMax = banSum;
                        heroBan = k;
                    }
                }
                // Find best hero deck (villain ban)
                banSumMax = 0;
                for (k = 0; k < 4; k++)
                {
                    banSum = 0;
                    for (l = 0; l < 4; l++)
                    {
                        banSum += matchupMatrix[allLineups[k][i]][fieldLineups[l][j]];
                    }
                    if (banSum > banSumMax)
                    {
                        banSumMax = banSum;
                        villainBan = k;
                    }
                }
                // Populate winrate matrix, excluding banned decks
                pastBan1 = false;
                for (k = 0; k < 4; k++)
                {
                    if (k == villainBan)
                    {
                        pastBan1 = true;
                        continue;
                    }
                    pastBan2 = false;
                    for (l = 0; l < 4; l++)
                    {
                        if (l == heroBan)
                        {
                            pastBan2 = true;
                            continue;
                        }
                        matrix[k - pastBan1][l - pastBan2] = matchupMatrix[allLineups[k][i]][fieldLineups[l][j]];
                    }
                }
                // Calculate EWR for matchup, add to sum
                wrSum += calcEWR3x3(matrix);
            }
            // Populate result array
            EWRs[i] = wrSum / fieldSize;
        }
    }
}

// Quicksort implementation
int partition(float EWRs[], int allLineups[][NUM_LINEUPS], int low, int high, int decksPerLU)
{
    float pivot = EWRs[low], tempFloat;
    int i = low - 1, j = high + 1, k, tempInt;

    while (1)
    {
        do
        {
            i++;
        } while (EWRs[i] > pivot);
        do 
        {
            j--;
        } while (EWRs[j] < pivot);
        
        if (i >= j)
        {
            return j;
        }
        tempFloat = EWRs[i];
        EWRs[i] = EWRs[j];
        EWRs[j] = tempFloat;
        for (k = 0; k < decksPerLU; k++)
        {
            tempInt = allLineups[k][i];
            allLineups[k][i] = allLineups[k][j];
            allLineups[k][j] = tempInt;
        }
    }
}
void quickSort(float EWRs[], int allLineups[][NUM_LINEUPS], int low, int high, int decksPerLU)
{
    if (low < high)
    {
        int pi = partition(EWRs, allLineups, low, high, decksPerLU);
        quickSort(EWRs, allLineups, low, pi, decksPerLU);
        quickSort(EWRs, allLineups, pi + 1, high, decksPerLU);
    }
}

// Function to output the sorted array of lineups and their respective EWRs
void printResults(const char *filename, float EWRs[], int allLineups[][NUM_LINEUPS], int numLineups, char index[STR_SIZE][NUM_DECKS], int decksPerLU)
{
    FILE *fp;
    int i, j;

    fopen_s(&fp, filename, "w");
    if (fp == NULL)
    {
        perror("error opening output file");
        exit(EXIT_FAILURE);
    }
    for (i = 0; i < numLineups; i++)
    {
        for (j = 0; j < decksPerLU; j++)
        {
            fprintf(fp, "%s", index[allLineups[j][i]]);
            if (j < decksPerLU - 1)
            {
                fprintf(fp, ", ");
            }
        }
        fprintf(fp, "\t%f\n", EWRs[i]);
    }

    if (fclose(fp) != 0) 
    {
        perror("error closing output file");
        exit(EXIT_FAILURE);
    }
}

// TODO: populate allLineups (no duplicate classes), EWRs (with ban), sort both & output
//       functionality for both bo3 and bo5 (bo7 later), conquest (lhs later), flat (single-elim, swiss, double-elim later)
//       other ideas for later: closed-list before/after class showdown
int main() 
{
    int bestOf, decksPerLU;
    int numDecksActual, fieldSizeActual, numLineupsActual;
    float matchupMatrix[NUM_DECKS][NUM_DECKS];
    char matrixIndex[NUM_DECKS][STR_SIZE];

    scanf_s("%d", &bestOf);


    decksPerLU = (bestOf + 3) / 2;

    int(*allLineups)[NUM_LINEUPS] = malloc(decksPerLU * sizeof(int* [NUM_LINEUPS]));
    int(*fieldLineups)[FIELD_SIZE] = malloc(decksPerLU * sizeof(int* [FIELD_SIZE]));

    // Get data from input files
    numDecksActual = readMatchups(MATCHUPS_FILENAME, matchupMatrix, matrixIndex);
    fieldSizeActual = readField(FIELD_FILENAME, fieldLineups, decksPerLU, matrixIndex, numDecksActual);
    // Generate all possible lineups
    numLineupsActual = makeLineups(allLineups, decksPerLU, matrixIndex, numDecksActual);

    float(*EWRs) = malloc(numLineupsActual * sizeof(float*));

    // Find EWR for all generated lineups in the given field
    calcAllEWRs(EWRs, numLineupsActual, allLineups, decksPerLU, fieldLineups, fieldSizeActual, matchupMatrix);
    // Sort all generated lineups by EWR
    quickSort(EWRs, allLineups, 0, numLineupsActual - 1, decksPerLU);
    // Output
    printResults(OUTPUT_FILENAME, EWRs, allLineups, numLineupsActual, matrixIndex, decksPerLU);

    free(allLineups);
    free(fieldLineups);
    free(EWRs);

    exit(EXIT_SUCCESS);
}