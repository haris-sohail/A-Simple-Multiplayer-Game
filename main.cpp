#include<iostream>
#include <SFML/Graphics.hpp>

using namespace std;
using namespace sf;

int generateRandom()
{
    // generating random number between 10 and 99
    srand(time(NULL));

    int randomNumber = rand() % 100;

    while(randomNumber < 10) // number should be between 10 and 99
    {
        randomNumber = rand() % 100;

        if(randomNumber >= 10)
        {
            break;
        }
    }

    // Roll No: 0531, therefore multiplication not needed

    int division = (531 / randomNumber) % 25;

    if(division < 10)
    {
        division += 15;
    }

    return division;
}

void createBoard(int n, int boxSize, int boardSizePixels)
{
    // create the textures first

    // create the window

    RenderWindow window(VideoMode(boardSizePixels, boardSizePixels), "Game Board");

    // create the square box

    RectangleShape squareShape(Vector2f(boxSize, boxSize)); // creates a 2D vector with length and width equal to the boxSize

    // create the game board

    for(int i = 0; i < n; i++)
    {
        for(int j = 0; j < n; j++)
        {
            // set the position of a particular box
            squareShape.setPosition(i * boxSize, j * boxSize); // this function takes x and y coordinates as pixel values

            // bicolor table functionality
            if((i + j) % 2 == 0)
            {
                squareShape.setFillColor(Color(64, 64, 64));
            }
            else
            {
                squareShape.setFillColor(Color(192, 192, 180));
            }

            // now draw the squareShape on the window
            window.draw(squareShape);
        }
    }

    // now display the board

    // display the board
    window.display();

    // run the window
    while (window.isOpen())
    {
        Event event;

        while (window.pollEvent(event))
        {
            if (event.type == Event::Closed) // when user crosses the screen or presses Alt+F4
            {
                window.close();
            }
        }
    }
}

int main()
{
    int n = generateRandom(); 

    // generate a n x n board 

    int boxSize = 60;

    // get the size of the window in pixels

    int boardSizePixels = n * 60;

    // creating the board
    createBoard(n, boxSize, boardSizePixels);
}