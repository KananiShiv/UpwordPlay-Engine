#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <stdbool.h>

#include "hw3.h"

#define DEBUG(...)                              \
    fprintf(stderr, "[          ] [ DEBUG ] "); \
    fprintf(stderr, __VA_ARGS__);               \
    fprintf(stderr, " -- %s()\n", __func__)

bool ValidPlacement(GameState *game, int row, int col, char direction, const char *tiles);
bool checkCoverExistingTiles(GameState *game, int row, int col, char direction, const char *tiles);

GameState *initialize_game_state(const char *filename)
{
    FILE *file = fopen(filename, "r");

    GameState *game = malloc(sizeof(GameState));

    game->rows = 0;
    game->cols = 0;
    int ch;
    int maxCols = 0;
    while ((ch = fgetc(file)) != EOF)
    {
        if (ch == '\n')
        {
            game->rows++;
            if (game->cols > maxCols)
            {
                maxCols = game->cols;
            }
            game->cols = 0;
        }
        else
        {
            game->cols++;
        }
    }
    game->cols = maxCols;
    game->board = malloc(game->rows * sizeof(char *));
    game->stackHeights = malloc(game->rows * sizeof(int *));
    for (int i = 0; i < game->rows; i++)
    {
        game->board[i] = malloc(game->cols * sizeof(char));
        game->stackHeights[i] = malloc(game->cols * sizeof(int));
    }

    rewind(file);
    int row = 0;
    int col = 0;
    while ((ch = fgetc(file)) != EOF)
    {
        if (ch == '\n')
        {
            while (col < game->cols)
            {
                game->board[row][col] = '.';
                game->stackHeights[row][col] = 0;
                col++;
            }
            row++;
            col = 0;
        }
        else
        {
            game->board[row][col] = ch;
            game->stackHeights[row][col] = (ch == '.') ? 0 : 1;
            col++;
        }
    }

    fclose(file);
    return game;
}

GameState *place_tiles(GameState *game, int row, int col, char direction, const char *tiles, int *num_tiles_placed)
{
    int placed = 0;

    if (row < 0 || col < 0 || row >= game->rows || col >= game->cols || (direction != 'H' && direction != 'V'))
    {
        return game;
    }

    // if (checkCoverExistingTiles(game, row, col, direction, tiles))
    // {
    //     // If the function returns true, it means you're attempting to cover existing tiles in a way you wanted to check.
    //     // You can handle this case as needed, perhaps aborting the tile placement or handling it differently.
    //     return game; // Example: Abort the operation
    // }

    // bool isValidPlacement = ValidPlacement(game, row, col, direction, tiles);

    // if (!isValidPlacement)
    // {
    //     return game;
    // }

    int neededRows;
    int neededCols;

    int tilesLength = 0;
    while (tiles[tilesLength] != '\0')
    {
        tilesLength++;
    }

    if (direction == 'V')
    {
        neededRows = row + tilesLength;
        neededCols = col + 1;
    }
    else if (direction == 'H')
    {
        neededCols = col + tilesLength;
        neededRows = row + 1;
    }
    if (neededCols > game->cols)
    {
        char **newBoard = malloc(game->rows * sizeof(char *));
        int **newStackHeights = malloc(game->rows * sizeof(int *));

        for (int i = 0; i < game->rows; i++)
        {
            newBoard[i] = malloc(neededCols * sizeof(char));
            newStackHeights[i] = malloc(neededCols * sizeof(int));

            for (int j = 0; j < neededCols; j++)
            {
                if (j < game->cols)
                {
                    newBoard[i][j] = game->board[i][j];
                    newStackHeights[i][j] = game->stackHeights[i][j];
                }
                else
                {
                    newBoard[i][j] = '.';
                    newStackHeights[i][j] = 0;
                }
            }
        }
        if (game->board != NULL)
        {
            for (int i = 0; i < game->rows; i++)
            {
                if (game->board[i] != NULL)
                {
                    free(game->board[i]);
                }
            }
            free(game->board);
            game->board = NULL;
        }

        if (game->stackHeights != NULL)
        {
            for (int i = 0; i < game->rows; i++)
            {
                if (game->stackHeights[i] != NULL)
                {
                    free(game->stackHeights[i]);
                }
            }
            free(game->stackHeights);
            game->stackHeights = NULL;
        }

        game->board = newBoard;
        game->stackHeights = newStackHeights;
        game->cols = neededCols;
    }
    if (neededRows > game->rows)
    {
        int newRows = neededRows;

        char **newBoard = malloc(newRows * sizeof(char *));

        int **newStackHeights = malloc(newRows * sizeof(int *));

        for (int i = 0; i < newRows; i++)
        {
            if (i < game->rows)
            {
                newBoard[i] = malloc(game->cols * sizeof(char));
                newStackHeights[i] = malloc(game->cols * sizeof(int));

                for (int j = 0; j < game->cols; j++)
                {
                    newBoard[i][j] = game->board[i][j];
                    newStackHeights[i][j] = game->stackHeights[i][j];
                }
            }
            else
            {
                newBoard[i] = malloc(game->cols * sizeof(char));
                newStackHeights[i] = malloc(game->cols * sizeof(int));

                for (int j = 0; j < game->cols; j++)
                {
                    newBoard[i][j] = '.';
                    newStackHeights[i][j] = 0;
                }
            }
        }
        if (game->board != NULL)
        {
            for (int i = 0; i < game->rows; i++)
            {
                if (game->board[i] != NULL)
                {
                    free(game->board[i]);
                }
            }
            free(game->board);
        }

        if (game->stackHeights != NULL)
        {
            for (int i = 0; i < game->rows; i++)
            {
                if (game->stackHeights[i] != NULL)
                {
                    free(game->stackHeights[i]);
                }
            }
            free(game->stackHeights);
        }

        game->board = newBoard;
        game->stackHeights = newStackHeights;
        game->rows = newRows;
    }
    for (int i = 0; tiles[i] != '\0'; ++i)
    {
        if (direction == 'H')
        {
            if (tiles[i] != ' ')
            {
                if ((game->board[row][col + i] == '.' && game->stackHeights[row][col + i] >= 5) ||
                    (game->board[row][col + i] != '.' && game->stackHeights[row][col + i] > 5))
                {
                    return game;
                }
                if (game->board[row][col + i] == '.' || game->stackHeights[row][col + i] < 5)
                {
                    game->board[row][col + i] = tiles[i];
                    game->stackHeights[row][col + i] += 1;
                    placed++;
                }
            }
        }
        else if (direction == 'V')
        {
            if (tiles[i] != ' ')
            {
                if ((game->board[row + i][col] == '.' && game->stackHeights[row + i][col] >= 5) ||
                    (game->board[row + i][col] != '.' && game->stackHeights[row + i][col] > 5))
                {
                    return game;
                }
                if (game->board[row + i][col] == '.' || game->stackHeights[row + i][col] < 5)
                {
                    game->board[row + i][col] = tiles[i];
                    game->stackHeights[row + i][col] += 1;
                    placed++;
                }
            }
        }
    }

    if (num_tiles_placed != NULL)
    {
        *num_tiles_placed = placed;
    }
    printf("dONE\n");

    return game;
}

bool checkCoverExistingTiles(GameState *game, int row, int col, char direction, const char *tiles)
{
    int i = 0;
    bool onlyCoversExistingTiles = true; // Assume initially that we only cover existing tiles.

    while (tiles[i] != '\0')
    {
        if (direction == 'H')
        {
            if (col + i >= game->cols || game->board[row][col + i] == '.')
            {
                onlyCoversExistingTiles = false; // Encountered an empty space or out of bounds, not covering existing tiles only.
                break;
            }
            if (tiles[i] == ' ' || tiles[i] != game->board[row][col + i])
            {
                onlyCoversExistingTiles = false; // Not matching or attempting to skip, so not covering existing tiles only.
                break;
            }
        }
        else if (direction == 'V')
        {
            if (row + i >= game->rows || game->board[row + i][col] == '.')
            {
                onlyCoversExistingTiles = false; // Encountered an empty space or out of bounds, not covering existing tiles only.
                break;
            }
            if (tiles[i] == ' ' || tiles[i] != game->board[row + i][col])
            {
                onlyCoversExistingTiles = false; // Not matching or attempting to skip, so not covering existing tiles only.
                break;
            }
        }
        i++;
    }

    // If onlyCoversExistingTiles is true, it means we are attempting to place tiles exactly over existing tiles without making a new word.
    return !onlyCoversExistingTiles; // Return false if we are only covering existing tiles, true otherwise.
}

bool ValidPlacement(GameState *game, int row, int col, char direction, const char *tiles)
{

    char simulatedWord[100];
    int wordIndex = 0;

    if (direction == 'V')
    {
        for (int i = 0; tiles[i] != '\0'; i++)
        {
            if (row + i >= game->rows)
            {
                simulatedWord[wordIndex++] = tiles[i];
                continue;
            }
            char currentTile = game->board[row + i][col];
            if (tiles[i] == ' ')
            {
                simulatedWord[wordIndex++] = currentTile;
            }
            else
            {
                simulatedWord[wordIndex++] = tiles[i];
            }
        }
    }
    else if (direction == 'H')
    {
        for (int i = 0; tiles[i] != '\0'; i++)
        {
            if (col + i >= game->cols)
            {
                simulatedWord[wordIndex++] = tiles[i];
                continue;
            }
            char currentTile = game->board[row][col + i];
            if (tiles[i] == ' ')
            {
                // if (currentTile == '.')
                // {
                //     return false;
                // }
                simulatedWord[wordIndex++] = currentTile;
            }
            else
            {
                simulatedWord[wordIndex++] = tiles[i];
            }
        }
    }

    simulatedWord[wordIndex] = '\0';

    FILE *file = fopen("tests/words.txt", "r");

    char fileWord[100];
    bool isValidWord = false;
    bool endOfLine = false;

    while (fgets(fileWord, sizeof(fileWord), file) != NULL)
    {
        for (int i = 0; i < (int)sizeof(fileWord); i++)
        {
            if (fileWord[i] == '\n')
            {
                fileWord[i] = '\0';
                endOfLine = true;
                break;
            }
        }

        if (!endOfLine && fileWord[sizeof(fileWord) - 1] != '\0')
        {
            int c;
            while ((c = fgetc(file)) != '\n' && c != EOF)
            {
                // Skip characters until the end of line
            }
        }

        int i = 0;
        while (simulatedWord[i] != '\0' && fileWord[i] != '\0')
        {
            if (simulatedWord[i] != fileWord[i])
            {
                break;
            }
            i++;
        }

        if (simulatedWord[i] == '\0' && fileWord[i] == '\0')
        {
            isValidWord = true;
            break;
        }
    }

    fclose(file);

    int tilesLength = 0;
    while (tiles[tilesLength] != '\0')
    {
        tilesLength++;
    }

    bool connectsToExistingTile = false;
    int newLettersPlaced = 0;
    bool attemptsToReplaceSameLetter = false;

    if (direction == 'H')
    {
        for (int i = 0; tiles[i] != '\0'; i++)
        {
            char currentTile;
            if (col + i < game->cols)
            {
                currentTile = game->board[row][col + i];
            }
            else
            {
                currentTile = '.';
            }
            if (currentTile == '.')
            {
                newLettersPlaced++;
            }
            else if (tiles[i] == ' ' && currentTile != '.')
            {
                connectsToExistingTile = true;
            }
            else if (tiles[i] != ' ' && currentTile != '.' && currentTile != tiles[i])
            {
                return false;
            }
            else if (tiles[i] != ' ' && currentTile == tiles[i])
            {
                attemptsToReplaceSameLetter = true;
            }
        }
    }
    else if (direction == 'V')
    {
        for (int i = 0; tiles[i] != '\0'; i++)
        {
            if (row + i >= game->rows)
            {
                simulatedWord[wordIndex++] = tiles[i];
                continue;
            }
            char currentTile = game->board[row + i][col];
            if (tiles[i] == ' ' && currentTile != '.')
            {
                connectsToExistingTile = true;
                simulatedWord[wordIndex++] = currentTile;
            }
            else if (tiles[i] != ' ')
            {
                if (currentTile == '.')
                {
                    newLettersPlaced++;
                    simulatedWord[wordIndex++] = tiles[i];
                }
                else if (currentTile == tiles[i])
                {
                    attemptsToReplaceSameLetter = true;
                    simulatedWord[wordIndex++] = tiles[i];
                }
                else
                {
                    return false;
                }
            }
            else if (tiles[i] == ' ' && currentTile == '.')
            {
                return false;
            }
        }
    }
    if (attemptsToReplaceSameLetter && !isValidWord)
    {
        return false;
    }
    if (!connectsToExistingTile && newLettersPlaced == 0)
    {
        return false;
    }

    if (!connectsToExistingTile || newLettersPlaced == 0 || attemptsToReplaceSameLetter)
    {
        return false;
    }
    if (!isValidWord)
    {
        return false;
    }
    return true;
}

GameState *undo_place_tiles(GameState *game)
{

    return game;
}

void free_game_state(GameState *game)
{

    if (game->board != NULL)
    {
        for (int i = 0; i < game->rows; i++)
        {
            if (game->board[i] != NULL)
            {
                free(game->board[i]);
            }
        }
        free(game->board);
    }

    if (game->stackHeights != NULL)
    {
        for (int i = 0; i < game->rows; i++)
        {
            if (game->stackHeights[i] != NULL)
            {
                free(game->stackHeights[i]);
            }
        }
        free(game->stackHeights);
    }
    free(game);
}

void save_game_state(GameState *game, const char *filename)
{
    FILE *file = fopen(filename, "w");

    for (int i = 0; i < game->rows; i++)
    {
        for (int j = 0; j < game->cols; j++)
        {
            fputc(game->board[i][j], file);
        }
        fprintf(file, "\n");
    }

    for (int i = 0; i < game->rows; i++)
    {
        for (int j = 0; j < game->cols; j++)
        {
            fprintf(file, "%d", game->stackHeights[i][j]);
        }
        fprintf(file, "\n");
    }

    fclose(file);
}