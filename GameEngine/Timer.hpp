class Timer {
public:
    Timer(float duration = 0.f, bool loop = false)
        : duration(duration), loop(loop) {}

    void Start()  { running = true; finished = false; elapsed = 0.f; }
    void Stop()   { running = false; }
    void Reset()  { elapsed = 0.f; finished = false; }

    void Update(float dt) {
        if (!running) return;

        elapsed += dt;

        if (elapsed >= duration) {
            finished = true;
            if (loop) elapsed = 0.f;
            else running = false;
        }
    }

    bool Finished() const { return finished; }

private:
    float duration;
    float elapsed = 0.f;
    bool running = false;
    bool loop = false;
    bool finished = false;
};
