
/**
 * Template array type.
 */
template <class X>
class kauto_array {
  X *ptr;
 
public:
  typedef X element_type;
  explicit kauto_array(size_t n) { ptr = new X[n]; }
  ~kauto_array() { delete [] ptr; }

  X& operator[](int n) { return ptr[n]; }
  operator X*() { return ptr; }
  X* operator +(int offset) { return ptr + offset; }

private:
  kauto_array& operator=(const kauto_array& a);
  kauto_array(const kauto_array& a);

};


