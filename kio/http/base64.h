#ifndef _BASE64_H
#define _BASE64_H

#ifdef __cplusplus
extern "C" {
#endif
  char *base64_encode_line(const char *s);
  char *base64_encode_string(const char *s, unsigned int len);
#ifdef __cplusplus
}
#endif

#endif
