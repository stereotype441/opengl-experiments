#include "file_tools.h"

#include <sys/types.h>
#include <sys/stat.h>

MappedFile::MappedFile(char const *filename)
{
  m_fd = open(filename, O_RDONLY);
  if (m_fd == -1) {
    throw MapException();
  }
  struct stat stats;
  if (fstat(m_fd, &stats) == -1) {
    close(m_fd);
    throw MapException();
  }
  m_size = stats.st_size;
  m_data = mmap(NULL, m_size, PROT_READ, MAP_PRIVATE, m_fd, 0);
  if (m_data == MAP_FAILED) {
    close(m_fd);
    throw MapException();
  }
}
