#include "main.h"
#include "libxml/tree.h"
#include "libxml/xmlstring.h"
#include <string.h>

int parse_stream_attributes (xmlNode *node, struct initial_stream *stream_attr)
{
        xmlAttr *attr;
        attr = node->properties;

        while(attr)
        {
                xmlChar* attr_value = xmlNodeListGetString(node->doc, attr->children, 1);
                if(attr_value)
                {
                        if (xmlStrcmp(attr->name, (const xmlChar *)"from") == 0)
                                strncpy(stream_attr->from, (const char *)attr_value, MAX_VALUE - 1);
                        else if (xmlStrcmp(attr->name, (const xmlChar *)"to") == 0)
                                strncpy(stream_attr->to, (const char *)attr_value, MAX_VALUE - 1);
                        else if (xmlStrcmp(attr->name, (const xmlChar *)"version") == 0)
                                strncpy(stream_attr->version, (const char *)attr_value, MAX_VALUE - 1);
                        else if (xmlStrcmp(attr->name, (const xmlChar *)"xml:lang") == 0)
                                strncpy(stream_attr->xml_lang, (const char *)attr_value, MAX_VALUE - 1);
                        else if (xmlStrcmp(attr->name, (const xmlChar *)"xmlns") == 0)
                                strncpy(stream_attr->xmlns, (const char *)attr_value, MAX_VALUE - 1);
                        else if (xmlStrcmp(attr->name, (const xmlChar *)"xmlns:stream") == 0)
                                strncpy(stream_attr->xmlns_stream, (const char *)attr_value, MAX_VALUE - 1);
                        xmlFree(attr_value);
                }
                attr = attr->next;
        }
        return 0;
}
