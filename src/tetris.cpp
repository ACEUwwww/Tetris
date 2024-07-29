/* Written by weisong4
   for the purpose of practicing 
   2024/7/28              */

#include <iostream>
#include <thread>
#include <vector>
#include <stdio.h>
#include <Windows.h>
#include <string>
using namespace std;
int field_width = 12;
int field_height = 18;
int ScreenWidth = 120;
int ScreenHeight = 30;
unsigned char *field_ptr = nullptr;
bool gameflag;
wstring square[7];

// Do the rotatation of coordinates in square
int rotate(int px, int py, int r){
    switch(r % 4){
        case 0: return py*4 + px; // 0 for 0 degree
        case 1: return 12 + py - 4*px; // 1 for 90 degrees
        case 2: return 15 - 4*py -px; // 2 for 180 degrees
        case 3: return 3 - py + px*4; // 3 for 270 degrees
    };
    return 0;
}

// pos_x and pos_y stand for the top left element in the square
int collision_detect(int pos_x,int pos_y,int rotation_state, int piece_number){
    // pointer in square
    for (int px = 0; px < 4; px++){
        for (int py = 0; py < 4; py++){
            int pi = rotate(px,py,rotation_state);
            int fi = (pos_y+py)*field_width + (pos_x+px);
            if (pos_x+px >= 0 && pos_x+px < field_width)
                if(pos_y+py >= 0 && pos_y+py < field_height)
                    if (field_ptr[fi] != 0 && square[piece_number][pi] != L'.')
                        return false;
        }
    }
    return true;
}
/* Judge whether the line is full given py */
bool isLineFull(int py){
    for (int px = 1; px < field_width-1; px ++){
        int fi = py*field_width + px;
        if(field_ptr[fi] == 0) return false;
    }
    return true;
}

int vertical_scroll(int pos_y){
    for (int py = pos_y; py > 0; py--){
        for (int px = 1; px < field_width-1; px++){
            field_ptr[py*field_width+px] = field_ptr[(py-1)*field_width+px];
        }
    }
    for (int px = 1; px < field_width-1;px++){
        field_ptr[px] = 0;
    }
    return 0;
}

/* Draw the block on the bottom to the field so that it can be contained by the screen */
int draw_square(int current_x,int current_y,int rotation_state, int piece_number){
    for(int px = 0; px < 4; px++){
        for (int py = 0; py < 4; py++){
            int pi = rotate(px,py,rotation_state);
            int fi = (current_y+py)*field_width + (current_x+px);
            if (current_x+px >= 0 && current_x+px < field_width)
                if(current_y+py >= 0 && current_y+py < field_height)
                    if (field_ptr[fi] == 0 && square[piece_number][pi] == L'X')
                        field_ptr[fi] = piece_number + 1;
        }
    }
    return 0;
}

/* Get a random piece number of [min,max] */
int getRand(int min, int max) {
    return ( rand() % (max - min + 1) ) + min ;
}

int main(){
    // Create Screen Buffer
	wchar_t *screen = new wchar_t[ScreenWidth*ScreenHeight];
	for (int i = 0; i < ScreenWidth*ScreenHeight; i++) screen[i] = L' ';
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;

    //Create square
    square[0].append(L"..X.");
    square[0].append(L"..X.");
    square[0].append(L"..X.");
    square[0].append(L"..X.");

    square[1].append(L"....");
    square[1].append(L"....");
    square[1].append(L"..XX");
    square[1].append(L"..XX");

    square[2].append(L"....");
    square[2].append(L"..X.");
    square[2].append(L"..X.");
    square[2].append(L".XX.");

    square[3].append(L"....");
    square[3].append(L".X..");
    square[3].append(L".X..");
    square[3].append(L".XX.");

    square[4].append(L"....");
    square[4].append(L"..X.");
    square[4].append(L".XX.");
    square[4].append(L"..X.");

    square[5].append(L"....");
    square[5].append(L".XXX");
    square[5].append(L"..X.");
    square[5].append(L"..X.");

    square[6].append(L"....");
    square[6].append(L".X..");
    square[6].append(L".XX.");
    square[6].append(L"..X.");


    // Create field
    field_ptr = new unsigned char[field_height*field_width];
    for (int y = 0; y < field_height; y++)
        for (int x = 0; x < field_width; x++)
            field_ptr[y*field_width+x] = (x==0 || x==field_width-1 || y == field_height-1)?9:0;

    gameflag = true;
    
    /* Some status */
    int rotate_state = 0;
    int current_x = field_width/2;
    int current_y = 0;
    int current_piece = 0;
    bool bKey[4];
    int rotate_lock = false;
    int speed_down = 20;
    int speed_limit = 5;
    int speed_count = 0;
    int score = 0;

    /* Main game loop */
    while (gameflag){
        //Timing =======================================================
        this_thread::sleep_for(25ms); // One game tick
        //Input ========================================================
        for (int k = 0; k < 4; k++)								// R   L   D Z
			bKey[k] = (0x8000 & GetAsyncKeyState((unsigned char)("\x27\x25\x28Z"[k]))) != 0;

        //Game Logic ===================================================
        /* Keyboard logic */
        /* Left */
        if (bKey[1])
            if (collision_detect(current_x-1,current_y,rotate_state,current_piece))
                current_x--;
        /* Right */
        if (bKey[0])
            if (collision_detect(current_x+1,current_y,rotate_state,current_piece))
                current_x++;
        /* Down */
        if (bKey[2])
            if(collision_detect(current_x,current_y+1,rotate_state,current_piece))
                current_y++;
        /* Rotate */
        if (bKey[3]){
            if (!rotate_lock){
                if(collision_detect(current_x,current_y,rotate_state+1,current_piece))
                    rotate_state++;
                rotate_lock = true;
            }
        }else rotate_lock = false; 


        /* Bottom logic and game over condition*/
        if (!collision_detect(current_x,current_y+1,rotate_state,current_piece)){
            if(speed_count == speed_down){
                draw_square(current_x,current_y,rotate_state,current_piece);
                /* Judge full line condition */
                for (int py = 0; py < 4; py++){
                    int pos_y = current_y + py;
                    if (isLineFull(pos_y) && pos_y >= 0 && pos_y < field_height - 1){
                        for(int px = 1; px < field_width-1; px++)
                            field_ptr[pos_y*field_width+px] = 8;
                        for (int x = 1; x < field_width; x++)
                            for (int y = 0; y < field_height; y++)
                                screen[(y + 2)*ScreenWidth + (x + 2)] = L" ABCDEFG=#"[field_ptr[y*field_width+x]];
                        WriteConsoleOutputCharacterW(hConsole, screen, ScreenWidth * ScreenHeight, { 0,0 }, &dwBytesWritten);
                        this_thread::sleep_for(400ms);
                        vertical_scroll(pos_y);
                        score += 25;
                    }
                }
                if (current_y == 0) gameflag = false;
                current_x = field_width / 2;
                current_y = 0;
                rotate_state = 0;
                current_piece = 0;
                speed_count = 0;
                if (!collision_detect(current_x,current_y,rotate_state,current_piece)) gameflag = false;
            } else speed_count++;

        }
        else{
            if (speed_count == speed_down){current_y++; speed_count = 0;}
            else speed_count++;
        }
        // Render Output ===============================================
        swprintf_s(&screen[2 * ScreenWidth + field_width + 6], 16, L"SCORE: %8d", score);

        //Drawing ======================================================
        /* Draw the boundary */
        for (int x = 0; x < field_width; x++)
            for (int y = 0; y < field_height; y++)
                screen[(y + 2)*ScreenWidth + (x + 2)] = L" ABCDEFG=#"[field_ptr[y*field_width+x]];

        /* Draw the current piece */
        for (int px = 0; px < 4; px++){
            for (int py = 0; py < 4; py++){
                if (square[current_piece][rotate(px,py,rotate_state)] == L'X')
                    screen[(current_y+py+2)*ScreenWidth+current_x+px+2] = current_piece+65;
            }
        }
        
        WriteConsoleOutputCharacterW(hConsole, screen, ScreenWidth * ScreenHeight, { 0,0 }, &dwBytesWritten);
    }

    /* Gameover condition */
    CloseHandle(hConsole);
	cout << "Game Over!! Your score is :"<< score <<  endl;
	system("pause");
	return 0;

}