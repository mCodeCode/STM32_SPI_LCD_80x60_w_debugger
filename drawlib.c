#include <stdlib.h>
#include "gpio_spi_helper.h"
#include "st7735.h"
#include <math.h>


//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
// HELPER METHODS FOR DRAWING


//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//HELPER TO SWAP THE COORDINATES FOR DRAWING
// 1. Define a structure to hold the two values
typedef struct {
    int a;
    int b;
} SwapResult;

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
// 2. Implement the swap method returning the struct
SwapResult swapValues(int a, int b) {
    SwapResult result;
    result.a = b; // Put B into a's slot
    result.b = a; // Put A into b's slot
    return result;
}




//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
// Function to set the active drawing window to a specific single pixel (or area)
/*
Calling ST7735_SetAddressWindow(x0, y0, x1, y1) defines a rectangular bounding box inside the screen's RAM. Any color bytes you stream immediately afterward via RAMWR (0x2C) will fill that box pixel-by-pixel, wrapping automatically from left-to-right and top-to-bottom like a typewriter
*/
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
void ST7735_SetAddressWindow(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1) {

    x0 += ST7735_XSTART;
    x1 += ST7735_XSTART;
    y0 += ST7735_YSTART;
    y1 += ST7735_YSTART;


    // 1. Column Address Set Command (CASET = 0x2A)
    LCD_WriteCommand(0x2A);
    LCD_WriteData(0x00);
    LCD_WriteData(x0);    // Start X
    LCD_WriteData(0x00);
    LCD_WriteData(x1);    // End X

    // 2. Row Address Set Command (RASET = 0x2B)
    LCD_WriteCommand(0x2B);
    LCD_WriteData(0x00);
    LCD_WriteData(y0);    // Start Y
    LCD_WriteData(0x00);
    LCD_WriteData(y1);    // End Y

    // 3. Memory Write Command (RAMWR = 0x2C) - Tells screen color data is coming next
    LCD_WriteCommand(0x2C);
}





//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//DRAW METHODS

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
void ST7735_DrawPixel(uint8_t x, uint8_t y, uint16_t color){
    
    // SWAP THE COORDINATES FOR DRAWING
    SwapResult result = swapValues(x, y);
    x = result.a; 
    y = result.b;

    // Set the address window to create drawing area
    ST7735_SetAddressWindow(x, y, x, y);

    // Open the data stream and hold CS LOW for the entire bulk transfer
    LCD_StartData();

    // 4. Stream every pixel color back-to-back without toggling CS
    SPI1_SendByte(color >> 8);   // Send high byte
    SPI1_SendByte(color & 0xFF); // Send low byte

    // 5. Close the data stream (pulls CS High)
    LCD_EndData();

}



//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
void ST7735_DrawLine(unsigned char x0, unsigned char y0, unsigned char x1, unsigned char y1, unsigned short color){

    // SWAP THE COORDINATES FOR DRAWING
    SwapResult result = swapValues(x0, y0);
    x0 = result.a; 
    y0 = result.b;

    SwapResult result2 = swapValues(x1, y1);
    x1 = result2.a; 
    y1 = result2.b;

    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;
    
    while (1) {
        ST7735_DrawPixel(x0, y0, color);
        
        if (x0 == x1 && y0 == y1) {
            break;
        }
        
        int e2 = 2 * err;
        
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }

}

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------


//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
// Function to draw a filled rectangle/square (Optimized with continuous CS hold)
void ST7735_DrawFilledRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t color) {

    // SWAP THE COORDINATES FOR DRAWING
    SwapResult result = swapValues(x, y);
    x = result.a; 
    y = result.b;

    // 1. Set the address window to cover the entire width and height of the square
    // (Note: ST7735_SetAddressWindow ends with LCD_WriteCommand(0x2C) which is RAMWR)
    ST7735_SetAddressWindow(x, y, x + w - 1, y + h - 1);

    // 2. Calculate total number of pixels in the square
    uint32_t totalPixels = (uint32_t)w * h;

    // 3. Open the data stream and hold CS LOW for the entire bulk transfer
    LCD_StartData();

    // 4. Stream every pixel color back-to-back without toggling CS
    for (uint32_t i = 0; i < totalPixels; i++) {
      SPI1_SendByte(color >> 8);   // Send high byte
      SPI1_SendByte(color & 0xFF); // Send low byte
    }

    // 5. Close the data stream (pulls CS High)
    LCD_EndData();
}

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//3D SECTION
// QQQ


//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
// Project 3D point to 2D screen space using pure integer shifts to avoid float overhead
Vector3D ST7735_ProjectPoint(int32_t x, int32_t y, int32_t z) {
    Vector3D out;

    // SWAP THE COORDINATES FOR DRAWING
    SwapResult result = swapValues(x, y);
    x = result.a; 
    y = result.b;
    
    // Simple perspective projection denominator (prevent division by zero)
    // Adjust 'camera_z' depending on how far back you want the object
    int32_t camera_z = 80; 
    int32_t depth = z + camera_z;
    if (depth < 1) depth = 1;

    // Scale factor for perspective (multiplied by 64 for fixed-point precision)
    int32_t scale = 64; 

    // Projection math: project X and Y relative to screen center
    // Center of 80x160 is X=40, Y=80
    out.x = (int16_t)(ST7735_WIDTH / 2 + (x * scale / depth));
    out.y = (int16_t)(ST7735_HEIGHT / 2 + (y * scale / depth));
    out.z = (int16_t)z;

    return out;
}


//-----------------------------------------------------------
//-----------------------------------------------------------
//-----------------------------------------------------------
// Draw a single 3D-projected pixel safely with clipping bounds
void ST7735_Draw3DPoint(int32_t x, int32_t y, int32_t z, uint16_t color) {
    Vector3D pt = ST7735_ProjectPoint(x, y, z);

    // Screen bounds clipping check
    if (pt.x < 0 || pt.x >= ST7735_WIDTH || pt.y < 0 || pt.y >= ST7735_HEIGHT) {
        return; 
    }

    // Optional: Draw a 2x2 block so individual pixels are actually visible
    uint8_t size = 5;
    int16_t x2 = (pt.x + size < ST7735_WIDTH) ? (pt.x + size) : (ST7735_WIDTH - 1);
    int16_t y2 = (pt.y + size < ST7735_HEIGHT) ? (pt.y + size) : (ST7735_HEIGHT - 1);

    // Set address window (utilizing your working 2D hardware functions)
    ST7735_SetAddressWindow(pt.x, pt.y, x2, y2);

    LCD_StartData();
    
    uint32_t totalPixels = (x2 - pt.x + 1) * (y2 - pt.y + 1);
    for (uint32_t i = 0; i < totalPixels; i++) {
        SPI1_SendByte(color >> 8);   
        SPI1_SendByte(color & 0xFF); 
    }
    
    LCD_EndData();
}


//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------

// 1. The Vertex Array (8 corners of the cube)
// Vertex 0: (-10, -10, -10) -> Bottom-Back-Left
// Vertex 1: ( 10, -10, -10) -> Bottom-Back-Right
// Vertex 2: ( 10,  10, -10) -> Top-Back-Right
// Vertex 3: (-10,  10, -10) -> Top-Back-Left
// Vertex 4: (-10, -10,  10) -> Bottom-Front-Left
// Vertex 5: ( 10, -10,  10) -> Bottom-Front-Right
// Vertex 6: ( 10,  10,  10) -> Top-Front-Right
// Vertex 7: (-10,  10,  10) -> Top-Front-Left
// const Vector3D cubeVertices[8] = {
// {-1, -1, -1}, // 0
// { 1, -1, -1}, // 1
// { 1,  1, -1}, // 2
// {-1,  1, -1}, // 3
// {-1, -1,  1}, // 4
// { 1, -1,  1}, // 5
// { 1,  1,  1}, // 6
// {-1,  1,  1}  // 7
// };


// Index Array for Wireframe Lines (12 Edges):
// To draw a wireframe cube, we connect pairs of vertices. Each pair defines a line segment:
// Back face: 0-1, 1-2, 2-3, 3-0
// Front face: 4-5, 5-6, 6-7, 7-4
// Connecting edges: 0-4, 1-5, 2-6, 3-7
// const uint8_t cubeIndices[12][2] = {
// {0, 1}, {1, 2}, {2, 3}, {3, 0}, // Back square
// {4, 5}, {5, 6}, {6, 7}, {7, 4}, // Front square
// {0, 4}, {1, 5}, {2, 6}, {3, 7}  // Connecting edges
// };


// How to Render the Cube
// To draw the cube, loop through the index array, project both connected vertices onto the screen, and draw a line between them using a line-drawing algorithm


//-----------------------------------------------------------
//-----------------------------------------------------------
//-----------------------------------------------------------
//HELPERS FOR TRANSLATION AND ROTATION

//-----------------------------------------------------------
//-----------------------------------------------------------
//-----------------------------------------------------------
Vector3D translatePoint(Vector3D p, Vector3D offset){
    // Translate local vertices to the desired world starting position
    int32_t translatedX = p.x + offset.x;
    int32_t translatedY = p.y + offset.y;
    int32_t translatedZ = p.z + offset.z;

    Vector3D res = {translatedX, translatedY, translatedZ};
    return res; 
}


//-----------------------------------------------------------
//-----------------------------------------------------------
//-----------------------------------------------------------
Vector3D rotatePoint(Vector3D p, float cx, float cy, float cz, float sx, float sy, float sz){
    
    
    float x = (float)p.x;
    float y = (float)p.y;
    float z = (float)p.z;

    // 1. Rotate around Y-axis
    float x1 = x * cy + z * sy;
    float y1 = y;
    float z1 = -x * sy + z * cy;

    // 2. Rotate around X-axis
    float x2 = x1;
    float y2 = y1 * cx - z1 * sx;
    float z2 = y1 * sx + z1 * cx;

    // 3. Rotate around Z-axis (Optional, if you want Z rotation too)
    float x3 = x2 * cz - y2 * sz;
    float y3 = x2 * sz + y2 * cz;
    float z3 = z2;

    Vector3D res = {
        (int32_t)x3, 
        (int32_t)y3,
        (int32_t)z3
    }; 

    return res;
}


//-----------------------------------------------------------
//-----------------------------------------------------------
//-----------------------------------------------------------
typedef struct {
    float cx;
    float cy;
    float cz;
    float sx;
    float sy;
    float sz;
} anglesHelper;
//-----------------------------------------------------------
anglesHelper precomputeTrig(float angleX, float angleY, float angleZ){
    // Precompute sine and cos for angles ONCE per object (saves CPU time)
    float radX = angleX * (3.14159265f / 180.0f);
    float radY = angleY * (3.14159265f / 180.0f);
    float radZ = angleZ * (3.14159265f / 180.0f);

    float cx = cosf(radX), sx = sinf(radX);
    float cy = cosf(radY), sy = sinf(radY);
    float cz = cosf(radZ), sz = sinf(radZ);

    anglesHelper res = {
        cx,
        cy,
        cz,
        sx,
        sy,
        sz,
    };

    return res;
}
//-----------------------------------------------------------
//-----------------------------------------------------------
//-----------------------------------------------------------
void drawSegment(Vector3D v1, Vector3D v2, anglesHelper trig, int32_t startX, int32_t startY, int32_t startZ, uint16_t color){
 
        //rotate points according to selected angles
        Vector3D v1Rot = rotatePoint(v1, trig.cx, trig.cy, trig.cz, trig.sx, trig.sy, trig.sz);
        Vector3D v2Rot = rotatePoint(v2, trig.cx, trig.cy, trig.cz, trig.sx, trig.sy, trig.sz);

        // Translate local vertices to the desired world starting position
        Vector3D v1Tr = translatePoint(v1Rot, (Vector3D){startX, startY, startZ});
        Vector3D v2Tr = translatePoint(v2Rot, (Vector3D){startX, startY, startZ});

        // Project both 3D points into 2D screen space
        Vector3D p1 = ST7735_ProjectPoint(v1Tr.x, v1Tr.y, v1Tr.z);
        Vector3D p2 = ST7735_ProjectPoint(v2Tr.x, v2Tr.y, v2Tr.z);

        // Draw line from p1 to p2 on the ST7735 screen
        ST7735_DrawLine(p1.x, p1.y, p2.x, p2.y, color);
};
//-----------------------------------------------------------
//-----------------------------------------------------------
//-----------------------------------------------------------
void ST7735_DrawBoxWireFrame(int32_t startX, int32_t startY, int32_t startZ, int32_t w, int32_t h, int32_t d, float angleX, float angleY, float angleZ, uint16_t color){

    // Half-dimensions to center the local shape around its own local origin (0,0,0)
    int32_t hw = w / 2;
    int32_t hh = h / 2;
    int32_t hd = d / 2;

    //vertex array
    //Define the 8 local vertices relative to (0,0,0)
    Vector3D cubeVertices[8] = {
        {-hw, -hh, -hd}, // 0: Back-Bottom-Left
        { hw, -hh, -hd}, // 1: Back-Bottom-Right
        { hw,  hh, -hd}, // 2: Back-Top-Right
        {-hw,  hh, -hd}, // 3: Back-Top-Left
        {-hw, -hh,  hd}, // 4: Front-Bottom-Left
        { hw, -hh,  hd}, // 5: Front-Bottom-Right
        { hw,  hh,  hd}, // 6: Front-Top-Right
        {-hw,  hh,  hd}  // 7: Front-Top-Left
    };

    //index array
    uint8_t cubeIndices[12][2] = {
        {0, 1}, {1, 2}, {2, 3}, {3, 0}, // Back square
        {4, 5}, {5, 6}, {6, 7}, {7, 4}, // Front square
        {0, 4}, {1, 5}, {2, 6}, {3, 7}  // Connecting edges
    };


    //-------------------------------
    //-------------------------------
    //-------------------------------
    // Precompute sine and cos for angles ONCE per object (saves CPU time)
    anglesHelper trig = precomputeTrig(angleX, angleY, angleZ);

    //Loop through indices, translate to world position, project, and draw
    for (int i = 0; i < 12; i++) {

        // Get the two vertices that make up this edge
        Vector3D v1 = cubeVertices[cubeIndices[i][0]];
        Vector3D v2 = cubeVertices[cubeIndices[i][1]];

        //draw that part of the shape
        drawSegment(v1, v2,trig, startX, startY, startZ, color);
    }

}

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------

// to draw a piramid you need a square for the base, 
    //and a point at the center of that square, elevated a certain height
    /*
    
      *  ---> point at height H
     / \
    /   \
   /     \
  /       \
 /         \
/___________\


      *  ---> point at height H
     /| \
    / |  \
   /  |   \
  /   |    \
 /    |     \
/_____|______\

and from the base square, i draw a line from each of the corners of that square
and those lines will connect with the point at H drawing the piramid

    
    
    */
void ST7735_DrawPyramidWireFrame(int32_t startX, int32_t startY, int32_t startZ, int32_t w, int32_t h, int32_t d, float angleX, float angleY, float angleZ, uint16_t color){

    
    // Half-dimensions to center the local shape around its own local origin (0,0,0)
    int32_t hw = w / 2;
    int32_t hh = h / 2;
    int32_t hd = d / 2;

    // 5 vertices: 0 to 3 are the base plane, 4 is the tip at the center top
    Vector3D pyramidVertices[5] = {
        {-hw,  hh, -hd}, // 0: Back-Bottom-Left
        { hw,  hh, -hd}, // 1: Back-Bottom-Right
        { hw,  hh,  hd}, // 2: Front-Bottom-Right
        {-hw,  hh,  hd}, // 3: Front-Bottom-Left
        {  0, -hh,   0}  // 4: Tip (Center, shifted up by height)
    };

    // Base square indices (4 edges) + Sides connecting to the tip (4 edges)
    uint8_t pyramidIndices[8][2] = {
        {0, 1}, {1, 2}, {2, 3}, {3, 0}, // Base square
        {0, 4}, {1, 4}, {2, 4}, {3, 4}  // Edges connecting base vertices to the tip (4)
    };


    //-------------------------------
    //-------------------------------
    //-------------------------------

    // Precompute sine and cos for angles ONCE per object (saves CPU time)
    anglesHelper trig = precomputeTrig(angleX, angleY, angleZ);

    //Loop through indices, translate to world position, project, and draw
    for (int i = 0; i < 8; i++) {

        // Get the two vertices that make up this edge
        Vector3D v1 = pyramidVertices[pyramidIndices[i][0]];
        Vector3D v2 = pyramidVertices[pyramidIndices[i][1]];

        //draw that part of the shape
        drawSegment(v1, v2,trig, startX, startY, startZ, color);
        
    }

}

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
void ST7735_DrawOctahedronWireFrame(int32_t startX, int32_t startY, int32_t startZ, int32_t w, int32_t h, int32_t d, float angleX, float angleY, float angleZ, uint16_t color){

    
    // Half-dimensions to center the local shape around its own local origin (0,0,0)
    int32_t hw = w / 2;
    int32_t hh = h / 2;
    int32_t hd = d / 2;

    // 5 vertices: 0 to 3 are the base plane, 4 is the tip at the center top
    Vector3D pyramidVertices[6] = {
        {-hw,  hh, -hd}, // 0: Back-Bottom-Left
        { hw,  hh, -hd}, // 1: Back-Bottom-Right
        { hw,  hh,  hd}, // 2: Front-Bottom-Right
        {-hw,  hh,  hd}, // 3: Front-Bottom-Left
        {  0, -hh,   0},  // 4: Tip (Center, shifted up by height)
        {  0, 3 * hh,   0}  // 5: Second Tip (Center, shifted down by height)
    };

    // Base square indices (4 edges) + Sides connecting to the tip (4 edges)
    uint8_t pyramidIndices[12][2] = {
        {0, 1}, {1, 2}, {2, 3}, {3, 0}, // Base square
        {0, 4}, {1, 4}, {2, 4}, {3, 4},  // Edges connecting base vertices to the tip (4)
        {0, 5}, {1, 5}, {2, 5}, {3, 5}  // Edges connecting base vertices to the tip (5)
    };


    //-------------------------------
    //-------------------------------
    //-------------------------------

    // Precompute sine and cos for angles ONCE per object (saves CPU time)
    anglesHelper trig = precomputeTrig(angleX, angleY, angleZ);

    //Loop through indices, translate to world position, project, and draw
    for (int i = 0; i < 12; i++) {

        // Get the two vertices that make up this edge
        Vector3D v1 = pyramidVertices[pyramidIndices[i][0]];
        Vector3D v2 = pyramidVertices[pyramidIndices[i][1]];

        //draw that part of the shape
        drawSegment(v1, v2,trig, startX, startY, startZ, color);
        
    }

}


//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
// 3.14159265f

void ST7735_DrawDodecahedronWireFrame(int32_t startX, int32_t startY, int32_t startZ, int32_t radius, float angleX, float angleY, float angleZ, uint16_t color) {
    anglesHelper trig = precomputeTrig(angleX, angleY, angleZ);
    
    // Golden ratio scaling factors for a regular dodecahedron
    int32_t a = (radius * 57) / 100;
    int32_t b = (radius * 35) / 100;
    int32_t c = (radius * 95) / 100;

    // 20 vertices of a regular dodecahedron
    Vector3D dodecVerts[20] = {
        // 8 cube vertices
        {  a,  a,  a }, { -a,  a,  a }, {  a, -a,  a }, { -a, -a,  a },
        {  a,  a, -a }, { -a,  a, -a }, {  a, -a, -a }, { -a, -a, -a },
        // 12 rectangular frame vertices
        {  0,  b,  c }, {  0, -b,  c }, {  0,  b, -c }, {  0, -b, -c },
        {  c,  0,  b }, { -c,  0,  b }, {  c,  0, -b }, { -c,  0, -b },
        {  b,  c,  0 }, { -b,  c,  0 }, {  b, -c,  0 }, { -b, -c,  0 }
    };

    // 30 exact edge connections forming the 12 pentagonal faces
    uint8_t dodecIndices[30][2] = {
        {0, 8}, {0, 12}, {0, 16},
        {1, 8}, {1, 13}, {1, 17},
        {2, 9}, {2, 12}, {2, 18},
        {3, 9}, {3, 13}, {3, 19},
        {4, 10}, {4, 14}, {4, 16},
        {5, 10}, {5, 15}, {5, 17},
        {6, 11}, {6, 14}, {6, 18},
        {7, 11}, {7, 15}, {7, 19},
        {8, 9}, {10, 11},
        {12, 14}, {13, 15},
        {16, 17}, {18, 19}
    };

    for (int i = 0; i < 30; i++) {
        Vector3D v1 = dodecVerts[dodecIndices[i][0]];
        Vector3D v2 = dodecVerts[dodecIndices[i][1]];

        drawSegment(v1, v2, trig, startX, startY, startZ, color);
    }
}

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
