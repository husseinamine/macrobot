class Mutex {
  public:
    Mutex();
    void lock();
    void unlock();
  private:
    bool state;
};
