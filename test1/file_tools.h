#pragma once

#include <cstddef>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>

class MapException
{
};

class MappedFile
{
public:
  MappedFile(char const *filename);

  void *data() const
  {
    return m_data;
  }

  ~MappedFile()
  {
    munmap(m_data, m_size);
    close(m_fd);
  }

private:
  int m_fd;
  size_t m_size;
  void *m_data;
};
