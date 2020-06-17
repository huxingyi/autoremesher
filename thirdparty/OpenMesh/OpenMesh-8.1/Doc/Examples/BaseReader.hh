class BaseReader
{
public:

  virtual std::string get_description() const = 0;
  virtual std::string get_extensions() const = 0;
  virtual std::string get_magic() const { return std::string(""); }

  virtual bool read(std::istream& _is, BaseImporter& _bi) const = 0;
  virtual bool read(const std::string& _filename, BaseImporter& _bi) const = 0;

  ...
};
