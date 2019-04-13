
// buffers from microphone
int * top1, * left, * top2, * right;

// frame size for microphone
const int FRAMELENGTH;

// sample rate (for timing calculation)
const int SAMPLERATE;

// functions
void readMic(int frameSize);

int *normalize(int *buffer); // returns normalized buffer

int findZero(int *buffer); // returns 0 on success

float calcAngle(int *top_buf, int *side_buf); // returns angle in degrees

void updatePosition(float del1, float del2, float del3); // sets servos and LED


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
        }else if(!findZero(leftNorm)){
            continue;
        }else if(!findZero(top2Norm)){
            continue;
        }else if(!findZero(rightNorm)){
            continue;
        }

        // 4) calculate angles
        float delTopLeft = calcAngle(top1Norm, leftNorm);
        float delTopRight = calcAngle(top2Norm, rightNorm);
        float delLeftRight = calcAngle(leftNorm, rightNorm);

        // 5) update position and turn on LED
        updatePosition(delTopLeft, delTopRight, delLeftRigth);

        // delay for a bit to prevent jittering
        // delay(1000);

    }
}




void readMic(int frameSize)
{
    // todo Kavi
}

int *normalize(int *buffer)
{
    //todo Roni
    static int *normalized;
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

// args -- (delay between top mic and left, delay between top mic and right, delay between left mic and right) in us
void updatePosition(float delay1, float delay2, float delay3)
{
    // todo Try Kavi
}
