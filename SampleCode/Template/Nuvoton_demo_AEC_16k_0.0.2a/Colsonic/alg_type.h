#ifndef ALG_TYPE_H
#define ALG_TYPE_H

/* IDs must match the USB baseline library and the binary parameter profile. */
typedef enum
{
    APM_MODULE = 0,
    BF_MODULE,
    PRE_AEC_MODULE,
    PRE_NS_MODULE,
    POST_AEC_MODULE,
    POST_NS_MODULE,
    EQ_MODULE,
    DRC_MODULE,
    AGC_MODULE,
    GAIN_MODULE,
    END_MODULE_TYPE_NUM
} module_id_type_t;

#endif
