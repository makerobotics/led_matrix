#ifndef _TABLES
#define _TABLES

static const uint8_t pupils_L[][2] = {
  {2,4}, // 0 - centre
  {1,4}, // 1 - right
  {3,4}, // 2 - left
  {2,5}, // 3 - bottom right
  {3,5}, // 4 - bottom left
  {3,3}, // 5 - GREEN centre
  {1,3}, // 6 - GREEN left
  {2,3}, // 7
  {4,3}, // 8
  {5,3}, // 9 - GREEN right
  {2,4}, // 10 - GREEN top left
  {3,4}, // 11
  {4,4}, // 12 - GREEN top right
  {3,5}, // 13 - GREEN top
  {2,2}, // 14
  {3,2}, // 15
  {4,2}, // 16
  {3,1}  // 17 - GREEN bottom
};

static const uint8_t pupils_R[][2] = {
  {4,4}, // 0 - centre
  {3,4}, // 1 - right
  {5,4}, // 2 - left
  {3,5}, // 3 - bottom right
  {4,5}, // 4 - bottom left
  {3,3}, // 5 - GREEN centre
  {1,3}, // 6 - GREEN left
  {2,3}, // 7
  {4,3}, // 8
  {5,3}, // 9 - GREEN right
  {2,4}, // 10 - GREEN top left
  {3,4}, // 11
  {4,4}, // 12 - GREEN top right
  {3,5}, // 13 - GREEN top
  {2,2}, // 14
  {3,2}, // 15
  {4,2}, // 16
  {3,1}  // 17 - GREEN bottom
};

//static const uint8_t PROGMEM // Bitmaps are stored in program memory
static const uint8_t blinkImg[][8] = {    // Eye animation frames
    // The NICE (green) eye, both left and right
    { B00111100,         // Fully open nice/green eye
        B01111110, // 0
        B11111111,
        B11111111,
        B11111111,
        B11111111,
        B01111110,
        B00111100 },
    { B00000000,
        B01111110, // 1
        B11111111,
        B11111111,
        B11111111,
        B11111111,
        B01111110,
        B00111100 },
    { B00000000,
        B00000000, // 2
        B00111100,
        B11111111,
        B11111111,
        B11111111,
        B00111100,
        B00000000 },
    { B00000000,
        B00000000, // 3
        B00000000,
        B00111100,
        B11111111,
        B01111110,
        B00011000,
        B00000000 },
    { B00000000,         // Fully closed nice/green eye
        B00000000, // 4
        B00000000,
        B00000000,
        B10000001,
        B01111110,
        B00000000,
        B00000000 },

    // The EVIL (red) eye, left
    { B00000000,         // Fully open evil/red eye
        B11000000, // 5
        B11111000,
        B01111111,
        B01111111,
        B00111111,
        B00111110,
        B00011100 },
    { B00000000,
        B00000000, // 6
        B11111000,
        B01111111,
        B01111111,
        B00111111,
        B00111110,
        B00011100 },
    { B00000000,
        B00000000, // 7
        B11111000,
        B01111111,
        B01111111,
        B00111111,
        B00111110,
        B00000000 },
    { B00000000,
        B00000000, // 8
        B00000000,
        B01111111,
        B01111111,
        B00111111,
        B00000000,
        B00000000 },
    { B00000000,         // Fully closed evil/red eye
        B00000000, // 9
        B00000000,
        B00000000,
        B01111111,
        B00000000,
        B00000000,
        B00000000 },
    // The EVIL (red) eye, right
    { B00000000,         // Fully open evil/red eye
        B00000011, // 10
        B00011111,
        B11111110,
        B11111110,
        B11111100,
        B01111100,
        B00111000 },
    { B00000000,
        B00000000, // 11
        B00011111,
        B11111110,
        B11111110,
        B11111100,
        B01111100,
        B00111000 },
    { B00000000,
        B00000000, // 12
        B00011111,
        B11111110,
        B11111110,
        B11111100,
        B01111100,
        B00000000 },
    { B00000000,
        B00000000, // 13
        B00000000,
        B11111110,
        B11111110,
        B11111100,
        B00000000,
        B00000000 },
    { B00000000,         // Fully closed evil/red eye
        B00000000, // 14
        B00000000,
        B00000000,
        B11111110,
        B00000000,
        B00000000,
        B00000000 }
};

const uint8_t Font[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Code for char  
  0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x20,  // Code for char !
  0x50, 0x50, 0x50, 0x00, 0x00, 0x00, 0x00,  // Code for char "
  0x50, 0x50, 0xf8, 0x50, 0xf8, 0x50, 0x0A,  // Code for char #
  0x20, 0x78, 0x80, 0x70, 0x08, 0xf0, 0x20,  // Code for char $
  0xc8, 0xc8, 0x10, 0x20, 0x40, 0x98, 0x98,  // Code for char %
  0x60, 0x90, 0x80, 0x78, 0x90, 0x90, 0x60,  // Code for char &
  0x10, 0x20, 0x40, 0x00, 0x00, 0x00, 0x00,  // Code for char '
  0x10, 0x20, 0x40, 0x40, 0x40, 0x20, 0x10,  // Code for char (
  0x40, 0x20, 0x10, 0x10, 0x10, 0x20, 0x40,  // Code for char )
  0x00, 0xa8, 0x70, 0x20, 0x70, 0xa8, 0x00,  // Code for char *
  0x00, 0x20, 0x20, 0xf8, 0x20, 0x20, 0x00,  // Code for char +
  0x00, 0x00, 0x00, 0x60, 0x60, 0x20, 0x40,  // Code for char ,
  0x00, 0x00, 0x00, 0xf8, 0x00, 0x00, 0x00,  // Code for char -
  0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x60,  // Code for char .
  0x10, 0x10, 0x20, 0x20, 0x20, 0x40, 0x40,  // Code for char /
  0x70, 0x88, 0xc8, 0xa8, 0x98, 0x88, 0x70,  // Code for char 0
  0x20, 0x60, 0x20, 0x20, 0x20, 0x20, 0x70,  // Code for char 1
  0x70, 0x88, 0x08, 0x10, 0x20, 0x40, 0xf8,  // Code for char 2
  0xf8, 0x08, 0x10, 0x30, 0x08, 0x88, 0x70,  // Code for char 3
  0x10, 0x30, 0x50, 0x90, 0xf8, 0x10, 0x10,  // Code for char 4
  0xf8, 0x80, 0xf0, 0x08, 0x08, 0x88, 0x70,  // Code for char 5
  0x30, 0x40, 0x80, 0xf0, 0x88, 0x88, 0x70,  // Code for char 6
  0xf8, 0x08, 0x10, 0x20, 0x20, 0x20, 0x20,  // Code for char 7
  0x70, 0x88, 0x88, 0x70, 0x88, 0x88, 0x70,  // Code for char 8
  0x70, 0x88, 0x88, 0x78, 0x08, 0x10, 0x60,  // Code for char 9
  0x00, 0x60, 0x60, 0x00, 0x60, 0x60, 0x00,  // Code for char :
  0x60, 0x60, 0x00, 0x60, 0x60, 0x20, 0x40,  // Code for char ;
  0x10, 0x20, 0x40, 0x80, 0x40, 0x20, 0x10,  // Code for char <
  0x00, 0x00, 0xf8, 0x00, 0xf8, 0x00, 0x00,  // Code for char =
  0x80, 0x40, 0x20, 0x10, 0x20, 0x40, 0x80,  // Code for char >
  0x70, 0x88, 0x08, 0x10, 0x20, 0x00, 0x20,  // Code for char ?
  0x70, 0x88, 0xa8, 0xd8, 0xb0, 0x80, 0x70,  // Code for char @
  0x20, 0x50, 0x88, 0x88, 0xf8, 0x88, 0x88,  // Code for char A
  0xf0, 0x88, 0x88, 0xf0, 0x88, 0x88, 0xf0,  // Code for char B
  0x70, 0x88, 0x80, 0x80, 0x80, 0x88, 0x70,  // Code for char C
  0xe0, 0x90, 0x88, 0x88, 0x88, 0x90, 0xe0,  // Code for char D
  0xf8, 0x80, 0x80, 0xf0, 0x80, 0x80, 0xf8,  // Code for char E
  0xf8, 0x80, 0x80, 0xf0, 0x80, 0x80, 0x80,  // Code for char F
  0x70, 0x88, 0x80, 0xb8, 0x88, 0x88, 0x78,  // Code for char G
  0x88, 0x88, 0x88, 0xf8, 0x88, 0x88, 0x88,  // Code for char H
  0x70, 0x20, 0x20, 0x20, 0x20, 0x20, 0x70,  // Code for char I
  0x08, 0x08, 0x08, 0x08, 0x18, 0x88, 0x70,  // Code for char J
  0x88, 0x90, 0xa0, 0xc0, 0xa0, 0x90, 0x88,  // Code for char K
  0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0xf8,  // Code for char L
  0x88, 0xd8, 0xa8, 0xa8, 0x88, 0x88, 0x88,  // Code for char M
  0x88, 0x88, 0xc8, 0xa8, 0x98, 0x88, 0x88,  // Code for char N
  0x70, 0x88, 0x88, 0x88, 0x88, 0x88, 0x70,  // Code for char O
  0xf0, 0x88, 0x88, 0x88, 0xf0, 0x80, 0x80,  // Code for char P
  0x70, 0x88, 0x88, 0x88, 0xa8, 0x98, 0x78,  // Code for char Q
  0xf0, 0x88, 0x88, 0xf0, 0xa0, 0x90, 0x88,  // Code for char R
  0x70, 0x88, 0x80, 0x70, 0x08, 0x88, 0x70,  // Code for char S
  0xf8, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,  // Code for char T
  0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x70,  // Code for char U
  0x88, 0x88, 0x88, 0x88, 0x88, 0x50, 0x20,  // Code for char V
  0x88, 0x88, 0x88, 0xa8, 0xa8, 0xa8, 0x50,  // Code for char W
  0x88, 0x88, 0x50, 0x20, 0x50, 0x88, 0x88,  // Code for char X
  0x88, 0x88, 0x88, 0x50, 0x20, 0x20, 0x20,  // Code for char Y
  0xf8, 0x08, 0x10, 0x20, 0x40, 0x80, 0xf8,  // Code for char Z
  0x70, 0x40, 0x40, 0x40, 0x40, 0x40, 0x70,  // Code for char [
  0x40, 0x40, 0x20, 0x20, 0x20, 0x10, 0x10,  // Code for char BackSlash
  0x70, 0x10, 0x10, 0x10, 0x10, 0x10, 0x70,  // Code for char ]
  0x20, 0x50, 0x88, 0x00, 0x00, 0x00, 0x00,  // Code for char ^
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8,  // Code for char _
  0x40, 0x20, 0x10, 0x00, 0x00, 0x00, 0x00,  // Code for char `
  0x00, 0x00, 0x70, 0x88, 0x88, 0x98, 0x68,  // Code for char a
  0x80, 0x80, 0xb0, 0xc8, 0x88, 0x88, 0xf0,  // Code for char b
  0x00, 0x00, 0x70, 0x88, 0x80, 0x80, 0x70,  // Code for char c
  0x08, 0x08, 0x68, 0x98, 0x88, 0x88, 0x78,  // Code for char d
  0x00, 0x00, 0x70, 0x88, 0xf8, 0x80, 0x70,  // Code for char e
  0x30, 0x40, 0xe0, 0x40, 0x40, 0x40, 0x40,  // Code for char f
  0x78, 0x88, 0x88, 0x98, 0x68, 0x08, 0x70,  // Code for char g
  0x80, 0x80, 0xb0, 0xc8, 0x88, 0x88, 0x88,  // Code for char h
  0x20, 0x00, 0x60, 0x20, 0x20, 0x20, 0x70,  // Code for char i
  0x10, 0x00, 0x30, 0x10, 0x10, 0x10, 0x60,  // Code for char j
  0x80, 0x80, 0x88, 0x90, 0xa0, 0xd0, 0x88,  // Code for char k
  0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x20,  // Code for char l
  0x00, 0x00, 0xd0, 0xa8, 0xa8, 0xa8, 0xa8,  // Code for char m
  0x00, 0x00, 0xb0, 0xc8, 0x88, 0x88, 0x88,  // Code for char n
  0x00, 0x00, 0x70, 0x88, 0x88, 0x88, 0x70,  // Code for char o
  0xf0, 0x88, 0x88, 0xc8, 0xb0, 0x80, 0x80,  // Code for char p
  0x78, 0x88, 0x88, 0x98, 0x68, 0x08, 0x08,  // Code for char q
  0x00, 0x00, 0xa0, 0xd0, 0x80, 0x80, 0x80,  // Code for char r
  0x00, 0x00, 0x70, 0x80, 0x70, 0x08, 0xf0,  // Code for char s
  0x40, 0x40, 0xf0, 0x40, 0x40, 0x40, 0x30,  // Code for char t
  0x00, 0x00, 0x88, 0x88, 0x88, 0x98, 0x68,  // Code for char u
  0x00, 0x00, 0x88, 0x88, 0x88, 0x50, 0x20,  // Code for char v
  0x00, 0x00, 0xa8, 0xa8, 0xa8, 0xa8, 0x50,  // Code for char w
  0x00, 0x00, 0x88, 0x50, 0x20, 0x50, 0x88,  // Code for char x
  0x88, 0x88, 0x88, 0x98, 0x68, 0x08, 0x70,  // Code for char y
  0x00, 0x00, 0xf8, 0x10, 0x20, 0x40, 0xf8,  // Code for char z
  0x10, 0x20, 0x20, 0x40, 0x20, 0x20, 0x10,  // Code for char {
  0x20, 0x20, 0x20, 0x00, 0x20, 0x20, 0x20,  // Code for char |
  0x40, 0x20, 0x20, 0x10, 0x20, 0x20, 0x40,  // Code for char }
  0x00, 0x00, 0x48, 0xb0, 0x00, 0x00, 0x00,  // Code for char ~
  0x70, 0x50, 0x50, 0x50, 0x50, 0x50, 0x70   // Code for char 
};

#endif
