
template <> struct binary< std::vector<bool> >
{

  typedef std::vector< bool >    value_type;
  typedef value_type::value_type elem_type;

  static const bool is_streamable = true;

  static size_t size_of(void) { return UnknownSize; }
  static size_t size_of(const value_type& _v)
  {
    return _v.size() / 8 + ((_v.size() % 8)!=0); 
  }

  static
  size_t store( std::ostream& _ostr, const value_type& _v, bool ) 
  {
    size_t bytes = 0;
    
    size_t N = _v.size() / 8;
    size_t R = _v.size() % 8;

    size_t        idx;      // element index
    unsigned char bits; // bitset

    for (idx=0; idx < N; ++idx)
    {
      bits = 0;
      bits = bits |  (_v[idx+0] ? 1 : 0);
      bits = bits | ((_v[idx+1] ? 1 : 0) << 1);
      bits = bits | ((_v[idx+2] ? 1 : 0) << 2);
      bits = bits | ((_v[idx+3] ? 1 : 0) << 3);
      bits = bits | ((_v[idx+4] ? 1 : 0) << 4);
      bits = bits | ((_v[idx+5] ? 1 : 0) << 5);
      bits = bits | ((_v[idx+6] ? 1 : 0) << 6);
      bits = bits | ((_v[idx+7] ? 1 : 0) << 7);
      _ostr << bits;
    }
    bytes = N;

    if (R)
    {
      bits = 0;
      switch(R)
      {
	case 7: bits = bits | ((_v[idx+6] ? 1 : 0) << 6);
	case 6: bits = bits | ((_v[idx+5] ? 1 : 0) << 5);
	case 5: bits = bits | ((_v[idx+4] ? 1 : 0) << 4);
	case 4: bits = bits | ((_v[idx+3] ? 1 : 0) << 3);
	case 3: bits = bits | ((_v[idx+2] ? 1 : 0) << 2);
	case 2: bits = bits | ((_v[idx+1] ? 1 : 0) << 1);
	case 1: bits = bits |  (_v[idx+0] ? 1 : 0);
      }
      _ostr << bits;
      ++bytes;
    }

    assert( bytes == size_of(_v) );

    return bytes;
  }

  static
  size_t restore( std::istream& _istr, value_type& _v, bool )
  {
    size_t bytes = 0;
    
    size_t N = _v.size() / 8;
    size_t R = _v.size() % 8;

    size_t        idx;  // element index
    unsigned char bits; // bitset

    for (idx=0; idx < N; ++idx)
    {
      _istr >> bits;
      _v[idx+0] = ((bits & 0x01)!=0);
      _v[idx+1] = ((bits & 0x02)!=0);
      _v[idx+2] = ((bits & 0x04)!=0);
      _v[idx+3] = ((bits & 0x08)!=0);
      _v[idx+4] = ((bits & 0x10)!=0);
      _v[idx+5] = ((bits & 0x20)!=0);
      _v[idx+6] = ((bits & 0x40)!=0);
      _v[idx+7] = ((bits & 0x80)!=0);
    }
    bytes = N;

    if (R)
    {
      _istr >> bits;
      for(; idx < _v.size(); ++idx)
        _v[idx] = (bits & (1 << (idx%8)))!=0;
      ++bytes;
    }

    return bytes;
  }
};
