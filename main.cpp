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
#include<string>
#include <SFML/Audio.hpp>
#include<time.h>

using namespace std;
using namespace sf;

char message1[256];

struct player1
{
    RenderWindow* window;
    Sprite* playerSrpite;
    bool change;
    mqd_t* mq;
    Sprite** points;

    player1(RenderWindow* windowVal, bool changeVal, mqd_t* mqVal, Sprite* playerSpriteVal)
    {
        window = windowVal;
        change = changeVal;
        mq = mqVal;
        playerSrpite = playerSpriteVal;
    }

    void setPoints(Sprite* point1, Sprite* point2, Sprite* point3, Sprite* point4, Sprite* point5, Sprite* point6)
    {
        points = new Sprite* [6];

        points[0] = point1;
        points[1] = point2;
        points[2] = point3;
        points[3] = point4;
        points[4] = point5;
        points[5] = point6;
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
    bool collision = false;

    char message[256];
    string collisionMsg;

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

            // collisions
            for(int i = 0; i < 6; i++)
            {
                if(player1_obj->playerSrpite->getGlobalBounds().intersects(player1_obj->points[i]->getGlobalBounds()))
                {
                    collisionMsg = "1collide" + to_string(i);

                    sprintf(message, collisionMsg.c_str());

                    mq_send(*(player1_obj->mq), message, strlen(message), 0);

                    exitThread = true;
                    collision = true;

                    break;
                }
            }

            if(collision == true)
            {
                break;
            }     

            if (event.type == Event::KeyPressed)
            {
                //cout << collision << endl;
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

    // sound 

    SoundBuffer theekHaiBuffer;

    if(!theekHaiBuffer.loadFromFile("assets/theek hai.wav"))
    {
        cout << "Error loading theek hai" << endl;
    }

    Sound theekhai;
    theekhai.setBuffer(theekHaiBuffer);

    SoundBuffer samajhGyaBuffer;

    if(!samajhGyaBuffer.loadFromFile("assets/samajh gaya.wav"))
    {
        cout << "Error loading samajh gya" << endl;
    }

    Sound samajhgya;
    samajhgya.setBuffer(samajhGyaBuffer);

    // game win

    SoundBuffer gameOver;

    if(!gameOver.loadFromFile("assets/smooth_criminal.wav"))
    {
        cout << "Error loading smooth criminal" << endl;
    }

    Sound smoothCriminal;
    smoothCriminal.setBuffer(gameOver);

    srand(time(NULL));

    // score 

    // font
    sf::Font font;
    font.loadFromFile("assets/blue_smiles.ttf");
    Vector2u window_size = window.getSize();

    // score player 1
    Text score_player1;
    score_player1.setFont(font);
    score_player1.setCharacterSize(40);
    score_player1.setPosition(window_size.x - 260, 10);
    score_player1.setString("Score P1: 0");
    score_player1.setFillColor(Color(139, 0, 0));

    int score_1 = 0;

    // score player 2
    Text score_player2;
    score_player2.setFont(font);
    score_player2.setCharacterSize(40);
    score_player2.setPosition(window_size.x - 260, 60);
    score_player2.setString("Score P2: 0");
    score_player2.setFillColor(Color(0, 0, 139));

    int score_2 = 0;

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
    Texture spriteTexture, point1Texture, point2Texture, point3Texture, point4Texture, point5Texture, point6Texture;

    Sprite sprite, point1, point2, point3, point4, point5, point6;

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

    // create the collectables

    // load the collectable images into the textures

    point1Texture.loadFromFile("assets/point1.png");
    point2Texture.loadFromFile("assets/point2.png");
    point3Texture.loadFromFile("assets/point3.png");
    point4Texture.loadFromFile("assets/point4.png");
    point5Texture.loadFromFile("assets/point5.png");
    point6Texture.loadFromFile("assets/point6.png");

    // set the sprite 

    point1.setTexture(point1Texture);
    point2.setTexture(point2Texture);
    point3.setTexture(point3Texture);
    point4.setTexture(point4Texture);
    point5.setTexture(point5Texture);
    point6.setTexture(point6Texture);   

    // set the scale of the collectables

    point1.setScale(3.6, 3.6);
    point2.setScale(3.6, 3.6);
    point3.setScale(3.6, 3.6);
    point4.setScale(3.6, 3.6);
    point5.setScale(3.6, 3.6);
    point6.setScale(3.6, 3.6); 

    // set the position of the collectables
    point1.setPosition(4 * 60, 4 * 60);
    point2.setPosition(10 * 60, 6 * 60);
    point3.setPosition(2 * 60, 15 * 60);
    point4.setPosition(10 * 60, 12 * 60);
    point5.setPosition(13 * 60, 3 * 60);
    point6.setPosition(14 * 60, 14 * 60);

    // draw the sprite

    window.draw(sprite);

    // draw the text

    window.draw(score_player1);

    window.draw(score_player2);

    // draw the collectables

    window.draw(point1);
    window.draw(point2);
    window.draw(point3);
    window.draw(point4);
    window.draw(point5);
    window.draw(point6);

    // now display
    window.display();

    // making message queue for player 1

    mqd_t* mq = createMessageQueue(); // message queue descriptor

    // making object for player 1

    player1 player1_obj(&window, 0, mq, &sprite);

    player1_obj.setPoints(&point1, &point2, &point3, &point4, &point5, &point6);

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

        // collisions

        if(strcmp(message1, "1collide0") == 0)
        {
            point1.setPosition(-100,-100);

            // increment the score
            score_1 += 5;
            score_player1.setString("Score P1: " + to_string(score_1));

            if(rand() % 2 == 0)
            {
                theekhai.play();
            }
            else
            {
                samajhgya.play();
            }
        }

        if(strcmp(message1, "1collide1") == 0)
        {
            point2.setPosition(-100,-100);

            // increment the score
            score_1 += 5;
            score_player1.setString("Score P1: " + to_string(score_1));

            if(rand() % 2 == 0)
            {
                theekhai.play();
            }
            else
            {
                samajhgya.play();
            }
        }

        if(strcmp(message1, "1collide2") == 0)
        {
            point3.setPosition(-100,-100);

            // increment the score
            score_1 += 5;
            score_player1.setString("Score P1: " + to_string(score_1));

            if(rand() % 2 == 0)
            {
                theekhai.play();
            }
            else
            {
                samajhgya.play();
            }
        }

        if(strcmp(message1, "1collide3") == 0)
        {
            point4.setPosition(-100,-100);

            // increment the score
            score_1 += 5;
            score_player1.setString("Score P1: " + to_string(score_1));

            if(rand() % 2 == 0)
            {
                theekhai.play();
            }
            else
            {
                samajhgya.play();
            }
        }

        if(strcmp(message1, "1collide4") == 0)
        {
            point5.setPosition(-100,-100);

            // increment the score
            score_1 += 10;
            score_player1.setString("Score P1: " + to_string(score_1));

            if(rand() % 2 == 0)
            {
                theekhai.play();
            }
            else
            {
                samajhgya.play();
            }
        }

        if(strcmp(message1, "1collide5") == 0)
        {
            point6.setPosition(-100,-100);

            // increment the score
            score_1 += 10;
            score_player1.setString("Score P1: " + to_string(score_1));

            if(rand() % 2 == 0)
            {
                theekhai.play();
            }
            else
            {
                samajhgya.play();
            }
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

        if (score_1 == 40) // when user crosses the screen or presses Alt+F4
        {
            Clock gameOverClock;

            Time timeLimit = seconds(5);

            cout << score_1 << endl;
            smoothCriminal.play();

            if (gameOverClock.getElapsedTime() >= timeLimit)
            {
                window.close();
            }
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
        window.draw(score_player1);
        window.draw(score_player2);
        window.draw(point1);
        window.draw(point2);
        window.draw(point3);
        window.draw(point4);
        window.draw(point5);
        window.draw(point6);
        window.display();
    }
    pthread_exit(0);
}