typedef struct {
    unsigned int on_time;
    unsigned int off_time;
    unsigned int note_no;
} note;

// This is the "tune" (actually just some vaguely harmonically related random
// notes because I am lazy)
// I gave up on the note ends and just left it to the note stealing algo

note notes[] = {
//  {start, end, note},
    {100, 3000, 30},
    {4000, 9000, 32},
    {10000, 19000, 30},
    {10001, 29000, 33},
    {15000, 35000, 37},
    {15001, 35000, 42},
    {25000, 29000, 27},
    {28000, 29000, 33},
    {35000, 45000, 37},
    {35001, 45000, 42},
    {40000, 49000, 30},
    {40001, 49000, 33},
    {55000, 55000, 37},
    {55001, 65000, 42},
    {65000, 69000, 27},
    {68000, 69000, 33},
    {75000, 85000, 37},
    {75001, 85000, 30},
    {80000, 19000, 42},
    {80001, 29000, 45},
    {85000, 35000, 49},
    {85001, 35000, 30},
    {95000, 29000, 39},
    {98000, 29000, 45},
    {105000, 45000, 37},
    {105001, 45000, 42},
    {110000, 49000, 30},
    {110001, 49000, 33},
    {110002, 49000, 18},
    {110003, 49000, 6},
    {115000, 55000, 37},
    {115001, 65000, 42},
    {130000, 69000, 27},
    {140000, 69000, 33},
    {150000, 85000, 37},
    {150001, 85000, 42},
    {155000, 45000, 37},
    {155001, 45000, 42},
    {160000, 49000, 30},
    {160001, 49000, 33},
    {165000, 55000, 37},
    {165001, 65000, 42},
    {170000, 69000, 27},
    {180000, 69000, 33},
    {190000, 85000, 37},
    {190001, 85000, 42},
};

unsigned int bass_notes[] = {6, 4, 9, 11};
