#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#define MSGSIZE 16
int nr1 = 1;
int nr2 = 2;
int nr3 = 3;
int matrix[8][8];
int matrix_size = 8;

bool setupMatrix(int size)
{
    for (int i = 0; i < size; i++)
        for (int j = 0; j < size; j++)
            matrix[i][j] = 0;
}

//Urmatoarele 3 functii sunt pentru verificarea si aplicarea deciziei jucatorului
bool columnFull(int columnIndex)
{

    if (matrix[0][columnIndex] != 0)
        return true;
    return false;
}

int firstEmptyRow(int columnIndex)
{

    for (int i = matrix_size - 1; i >= 0; i--)
        if (matrix[i][columnIndex] == 0)
            return i;
}

bool addInColumn(int columnIndex, int value)
{

    if (columnFull(columnIndex))
        return false;

    int row = firstEmptyRow(columnIndex);
    matrix[row][columnIndex] = value;
    return true;
}

//Urmatoarele functii sunt pentru verificarea jocului
bool latitude(int row, int column)
{
    int i = -1, j = +1;
    int color = matrix[row][column];
    int sum = 1;
    while (column + i >= 0 && matrix[row][column + i] == color)
    {
        sum++;
        i = i - 1;
    }
    while (column + j < matrix_size && matrix[row][column + j] == color)
    {
        sum++;
        j = j + 1;
    }
    if (sum >= 4)
        return true;
    return false;
}

bool longitude(int row, int column)
{
    int i = -1, j = +1;
    int color = matrix[row][column];
    int sum = 1;
    while (row + i >= 0 && matrix[row + i][column] == color)
    {
        sum++;
        i = i - 1;
    }
    while (row + j < matrix_size && matrix[row + j][column] == color)
    {
        sum++;
        j = j + 1;
    }
    if (sum >= 4)
        return true;
    return false;
}

bool diagonal(int row, int column)
{

    int color = matrix[row][column];

    int sum1 = 1;
    int i1 = -1, j1 = 1;
    while (row + i1 >= 0 && column + i1 >= 0 && matrix[row + i1][column + i1] == color)
    {
        i1 -= 1;
        sum1 += 1;
    }
    while (row + j1 < matrix_size && column + j1 < matrix_size && matrix[row + j1][column + j1] == color)
    {
        j1 += 1;
        sum1 += 1;
    }
    if (sum1 >= 4)
        return true;

    int sum2 = 1;
    int i2 = -1, j2 = 1;
    while (row + i2 >= 0 && column - i2 < matrix_size && matrix[row + i2][column - i2] == color)
    {
        i2 -= 1;
        sum2 += 1;
    }
    while (row + j2 < matrix_size && column - j2 >= 0 && matrix[row + j2][column - j2] == color)
    {
        j2 += 1;
        sum2 += 1;
    }
    if (sum2 >= 4)
        return true;

    return false;
}

bool fourInRow(int row, int column)
{

    if (diagonal(row, column))
        return true;
    if (latitude(row, column))
        return true;
    if (longitude(row, column))
        return true;
    return false;
}

int endGame()
{

    for (int i = 0; i < matrix_size; i++)
        for (int j = 0; j < matrix_size; j++)
            if (fourInRow(i, j))
                return matrix[i][j];
    return 0;
}

int main()
{
    int color = 1;
    while (endGame == 0)
    {
        if (color == 1)
            color = 2;
        else
            color = 1;
        printf("\nEnter your row: ");
        int row = getchar();
        printf("\nYou entered: ");
        putchar(c);

        addInColumn(row, color) for (int i = 0; i < matrix_size; i++)
        {
            for (int j = 0; j < matrix_size; j++)
                printf("%d     ", matrix[x][y]);
            printf("\n");
        }
    }
}
