#ifndef CONF_PARSER_H
#define CONF_PARSER_H


char* get_pwd();
char* get_conf_file(const char *file, const char *path);
struct sdp_dev *parse_conf_file(const char *file_name);


#endif
