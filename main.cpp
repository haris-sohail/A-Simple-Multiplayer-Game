#include<iostream>
#include <SFML/Graphics.hpp>
#include<pthread.h>
#include<stdlib.h>
#include<unistd.h>
#include<fstream>
#include<fcntl.h>
#include<sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <mqueue.h>
#include<string.h>

using namespace std;
using namespace sf;

char message1[256];

struct player1
{
    RenderWindow* window;
    bool change;
    mqd_t* mq;

    player1(RenderWindow* windowVal, bool changeVal, mqd_t* mqVal)
    {
        window = windowVal;
        change = changeVal;
        mq = mqVal;
    }
};

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

void* player1_thread(void* args)
{
    player1* player1_obj = (player1*) args;
    bool exitThread = false;

    char message[256];

    while (!exitThread)
    {
        // user interaction
        Event event;

        while (player1_obj->window->pollEvent(event))
        {
            if (event.type == Event::Closed) 
            {
                exitThread = true;
                player1_obj->window->close(); 
                break;
            }
            else if (event.type == Event::KeyPressed)
            {
                player1_obj->change = 1; // something has been written

                if(Keyboard::isKeyPressed(Keyboard::Left)) // left
                {
                    sprintf(message, "1left");

                    mq_send(*(player1_obj->mq), message, strlen(message), 0);

                    exitThread = true;
                }

                else if(Keyboard::isKeyPressed(Keyboard::Right)) // right
                {
                    sprintf(message, "1right");

                    mq_send(*(player1_obj->mq), message, strlen(message), 0);

                    exitThread = true;
                }

                else if(Keyboard::isKeyPressed(Keyboard::Up)) // up
                {
                    sprintf(message, "1up");

                    mq_send(*(player1_obj->mq), message, strlen(message), 0);

                    exitThread = true;
                }

                else if(Keyboard::isKeyPressed(Keyboard::Down)) // down
                {
                    sprintf(message, "1down");

                    mq_send(*(player1_obj->mq), message, strlen(message), 0);

                    exitThread = true;
                }
            }
        }
    }

    pthread_exit(0);
}


void create_player1_thread(player1* player1_obj)
{
    pthread_t p1;

    pthread_attr_t atr1;

    // iniitiate
    pthread_attr_init(&atr1);

    // making thread joinable
    pthread_attr_setdetachstate(&atr1, PTHREAD_CREATE_JOINABLE);

    pthread_create(&p1, &atr1, player1_thread, player1_obj);

    pthread_join(p1, NULL);

    return;
}

void* readFromQueue(void* args)
{
    player1* player1_obj = (player1*) args;
    char message[256];

    unsigned int priority;

    while (player1_obj->window->isOpen())
    {
        int bytesRead = mq_receive(*(player1_obj->mq), message, sizeof(message), &priority);

        if (bytesRead > 0) // there exists a message
        {
            message[bytesRead] = '\0';
            //cout << "Received message: " << message << endl;

            strcpy(message1, message);
        }
    }

    pthread_exit(0);
}

void create_readThread(player1* player1_obj)
{
    pthread_t read;

    pthread_attr_t atr1;

    // iniitiate
    pthread_attr_init(&atr1);

    // making thread joinable
    pthread_attr_setdetachstate(&atr1, PTHREAD_CREATE_JOINABLE);

    pthread_create(&read, &atr1, readFromQueue, player1_obj);

    return;
}


mqd_t* createMessageQueue()
{
    mqd_t* mq = new mqd_t; // descriptor for message queue

    // setting the attributes for the message queues

    struct mq_attr atr;
    
    atr.mq_maxmsg = 10; // max no of messages
    atr.mq_flags = 0;
    atr.mq_curmsgs = 0; // no of messages currently
    atr.mq_msgsize = 256; // max size of message

    mq_unlink("/player1_queue"); // if there is a queue of the same name

    *mq = mq_open("/player1_queue", O_CREAT | O_RDWR, 0666, &atr); // open the Q

    return mq;

    if (*mq == (mqd_t)-1)
    {
        cout << "Queue open failed" << endl;
        return nullptr;
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

            window.draw(squareShape);
        }
    }

    // create the sprite 

    // create a texture first
    Texture spriteTexture;

    Sprite sprite;

    // Failed to load image

    if(!spriteTexture.loadFromFile("assets/player1.png"))
    {
        cout << "Error. Could not load sprite" << endl;
    }

    else
    {
        // Successfully loaded image

        sprite.setTexture(spriteTexture);
    }

    sprite.setScale(3.6, 3.6); // making the sprite twice as bigger

    // draw the sprite

    window.draw(sprite);

    // now display the sprite and the board
    window.display();

    // making message queue for player 1

    mqd_t* mq = createMessageQueue(); // message queue descriptor

    // making object for player 1

    player1 player1_obj(&window, 0, mq);

    // display the window
    while (window.isOpen())
    {
        // user interaction
        Event event;

        while (window.pollEvent(event))
        {
            if (event.type == Event::Closed) // when user crosses the screen or presses Alt+F4
            {
                window.close();
            }
        }

        // player 1 thread function

        create_player1_thread(&player1_obj);

        // read from the message queue

        create_readThread(&player1_obj);

        //updates

        // moving player 1 sprite

        if(strcmp(message1, "1right") == 0)
        {
            sprite.move(60, 0);
        }

        else if(strcmp(message1, "1left") == 0)
        {
            sprite.move(-60, 0);
        }

        else if(strcmp(message1, "1up") == 0)
        {
            sprite.move(0, -60);
        }

        else if(strcmp(message1, "1down") == 0)
        {
            sprite.move(0, 60);
        }

        // if the sprite is going outside the border

        Vector2u windowSize = window.getSize();
        FloatRect spriteBounds = sprite.getGlobalBounds();
        Vector2f spritePosition = sprite.getPosition();

        if(spritePosition.x < 0) // left border
        {
            sprite.setPosition(0, spritePosition.y);
        }

        else if(spritePosition.x + spriteBounds.width > windowSize.x) // right border
        {
            sprite.setPosition(windowSize.x - spriteBounds.width, spritePosition.y);
        }
        else if(spritePosition.y < 0) // top border
        {
            sprite.setPosition(spritePosition.x, 0);
        }
        else if(spritePosition.y + spriteBounds.height > windowSize.y) // bottom border
        {
            sprite.setPosition(spritePosition.x, windowSize.y - spriteBounds.height);
        }

        // draw 
        window.clear();
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

                window.draw(squareShape);
            }
        }
        window.draw(sprite);
        window.display();
    }
    pthread_exit(0);
}