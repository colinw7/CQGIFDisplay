class CGIFFrame {
 private:
  CGenImage *image_;
  int        delay_;      // hundredths of a second
  int        dispose_;    // 0=unknown, 1=no, 2=background, 4=previous
  bool       user_input_; // click for next frame

 public:
  CGIFFrame(CGenImage *image)  :
   image_(image), delay_(0), dispose_(0), user_input_(false) {
  }

 ~CGIFFrame() { }

  CGenImage *getImage() const { return image_; }

  void setDelay(int delay) { delay_ = delay; }
  int  getDelay() const { return delay_; }

  void setDispose(int dispose) { dispose_ = dispose; }
  int  getDispose() const { return dispose_; }

  void setUserInput(bool user_input) { user_input_ = user_input; }
  bool getUserInput() const { return user_input_; }
};

class CGIFAnim {
 private:
  typedef std::vector<CGIFFrame *> FrameList;

  FrameList frames_;

 public:
  typedef FrameList::iterator       iterator;
  typedef FrameList::const_iterator const_iterator;

  CGIFAnim() : frames_() { }

 ~CGIFAnim() {
    for (int i = frames_.size() - 1; i >= 0; --i)
      delete frames_[i];
  }

  void add(CGIFFrame *frame) {
    frames_.push_back(frame);
  }

  iterator begin() { return frames_.begin(); }
  iterator end  () { return frames_.end  (); }

  int size() const { return frames_.size(); }

  CGIFFrame *operator[](int pos) { return frames_[pos]; }
};
