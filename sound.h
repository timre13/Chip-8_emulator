#ifndef SOUND_H
#define SOUND_H


class Beeper
{
private:
    bool m_couldInit = false;
    int m_sampleI = 0;

public:
    Beeper();

    void startBeeping() const;
    void stopBeeping() const;

    ~Beeper();
};

#endif /* SOUND_H */
