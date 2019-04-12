
// buffers from microphone
int * top1, left, top2, right;

// frame size for microphone
const int FRAMELENGTH;

// sample rate (for timing calculation)
const int SAMPLERATE;

// functions
void readMic(int frameSize);

int *normalize(int *buffer); // returns normalized buffer

int findZero(int *buffer); // returns 0 on success

float calcAngle(int *top_buf, int *side_buf); // returns angle in degrees

void updatePosition(float ang1, float ang2); // sets servos and LED


int main ()
{
    while (1)
    {
        // 1) read data from microphone
        readMic(FRAMELENGTH);

        // 2) normalize data around 0
        int *top1Norm = normalize(top1);
        int *leftNorm = normalize(left);
        int *top2Norm = normalize(top2);
        int *rightNorm = normalize(right);

        // 3) if we can find 0 crossings, calculate angles, else goto 1)
        if (!findZero(top1Norm)){
            continue;
        }elif(!findZero(leftNorm);){
            continue;
        }elif(!findZero(top2Norm))
            continue;
        }elif(!findZero(rightNorm)){
            continue;
        }

        // 4) calculate angles
        float angleTopLeft = calcAngle(top1Norm, leftNorm);
        float angleTopRight = calcAngle(top2Norm, rightNorm);

        // 5) update position and turn on LED
        updatePosition(angleTopLeft, angleTopRight);

        // delaay for a bit to prevent jittering
        // delay(1000);

    }
}




void readMic(frameSize)
{
    // todo Kavi
}

int *normalize(int *buffer);
{
    //todo Roni
    int normalized[FRAMELENGTH]
    return normalized;
}

int findZero(int *buffer)
{
    //todo Roni
    return 0;
}

float calcAngle(int *top_buf, int *side_buf)
{
    // todo Roni
    float angle;
    return angle;
}

void updatePosition(float ang1, float ang2)
{
    // todo Try Kavi
}
