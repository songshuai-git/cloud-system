#ifndef PROTOCOL_H
#define PROTOCOL_H

#include<string.h>

typedef unsigned int uint;
//typedef unsigned char uint8;

#define RGST_SUEESS "regist success"
#define RGST_FAIL "注册失败：用户名已存在"
#define LOGIN_SUCCESS "login success"
#define LOGIN_FAIL "login fail:no name or pswd error or name existed"
#define SEARCH_USR_NO " no such usr"
#define SEARCH_USR_ONLINE " is online"
#define SEARCH_USR_OFFLINE " is offline"

#define SYSTEAM_ERROR_FORMAL_PARAM "system error:formal param"
#define SYSTEAM_ERROR "system error:unknow error"

#define PERNAME_NOT_EXIST "such usr no exist"
#define PERNAME_NOT_ONLINE "such usr not online"
#define PERNAME_ALREADY_FRI "already be your friend"

#define ADD_FRIEND_AGREE "has agreed your request"
#define ADD_FRIEND_REJECT "has rejected your request"
#define DEL_FRIEND_SUCCESS "remove success"
#define DEL_FRIEND_FAIL "remove failed"

#define CREATE_DIR_PATH_NOT_EXIST "current path not exist"
#define CREATE_DIR_DIRNAME_EXIST "dir name exist"
#define CREATE_DIR_OK "create success"

#define DEL_DIR_OK "delete dir success"
#define DEL_DIR_IS_FILE "delete failed: is a file not a dir"
#define DEL_DIR_NOT_EXIST "delete failed: such dir not existed"

#define ENTER_DIR_OK "enter dir success"
#define ENTER_DIR_IS_FILE "enter dir failed: is a file not a dir"

#define UPLOAD_FILE_FAIL "upload file fail"
#define UPLOAD_FILE_OK "upload success"

#define DEL_FILE_OK "delete file success"
#define DEL_FILE_IS_DIR "delete failed: is a dir not a file"
#define DEL_FILE_NOT_EXIST "delete failed: such file not existed"

#define DOWNLOAD_FILE_OK "download success"
#define DOWNLOAD_FILE_IS_DIR "download file fail: is a dir not a file"

#define SHARE_FILE_OK "share success"

#define MOVE_FILE_OK "move file success"
#define MOVE_FILE_FAIL "move file fail: is a File not a Dir"

enum ENUM_PDU_TYPE{
    ENUM_PDU_TYPE_MIN = 0x00000000,

    ENUM_PDU_TYPE_RGST_REQUST,
    ENUM_PDU_TYPE_RGST_RESPOND,

    ENUM_PDU_TYPE_LOGIN_REQUST,
    ENUM_PDU_TYPE_LOGIN_RESPOND,

    ENUM_PDU_TYPE_ALL_ONLINE_REQUST,
    ENUM_PDU_TYPE_ALL_ONLINE_RESPOND,

    ENUM_PDU_TYPE_SEARCH_USR_REQUST,
    ENUM_PDU_TYPE_SEARCH_USR_RESPOND,

    ENUM_PDU_TYPE_ADD_FRIEND_REQUST,
    ENUM_PDU_TYPE_ADD_FRIEND_RESPOND,

    ENUM_PDU_TYPE_ADD_FRIEND_AGREE,
    ENUM_PDU_TYPE_ADD_FRIEND_REJECT,

    ENUM_PDU_TYPE_FLUSH_FRI_REQUST,
    ENUM_PDU_TYPE_FLUSH_FRI_RESPOND,

    ENUM_PDU_TYPE_DEL_FRI_REQUST,
    ENUM_PDU_TYPE_DEL_FRI_RESPOND,

    ENUM_PDU_TYPE_PRIVATE_CHAT_REQUST,
    ENUM_PDU_TYPE_PRIVATE_CHAT_RESPOND,

    ENUM_PDU_TYPE_GROUP_CHAT_REQUST,
    ENUM_PDU_TYPE_GROUP_CHAT_RESPOND,

    ENUM_PDU_TYPE_CREATE_DIR_REQUST,
    ENUM_PDU_TYPE_CREATE_DIR_RESPOND,

    ENUM_PDU_TYPE_FLUSH_DIR_REQUST,
    ENUM_PDU_TYPE_FLUSH_DIR_RESPOND,

    ENUM_PDU_TYPE_DEL_DIR_REQUST,
    ENUM_PDU_TYPE_DEL_DIR_RESPOND,

    ENUM_PDU_TYPE_RENAME_DIR_REQUST,
    ENUM_PDU_TYPE_RENAME_DIR_RESPOND,

    ENUM_PDU_TYPE_ENTER_DIR_REQUST,
    ENUM_PDU_TYPE_ENTER_DIR_RESPOND,

    ENUM_PDU_TYPE_UPLOAD_FILE_REQUST,
    ENUM_PDU_TYPE_UPLOAD_FILE_RESPOND,

    ENUM_PDU_TYPE_DOWNLOAD_FILE_REQUST,
    ENUM_PDU_TYPE_DOWNLOAD_FILE_RESPOND,

    ENUM_PDU_TYPE_DEL_FILE_REQUST,
    ENUM_PDU_TYPE_DEL_FILE_RESPOND,

    ENUM_PDU_TYPE_SHARE_FILE_REQUST,
    ENUM_PDU_TYPE_SHARE_FILE_RESPOND,

    ENUM_PDU_TYPE_SHARE_FILE_NOTICE_REQUST,
    ENUM_PDU_TYPE_SHARE_FILE_NOTICE_RESPOND,

    ENUM_PDU_TYPE_MOVE_FILE_REQUST,
    ENUM_PDU_TYPE_MOVE_FILE_RESPOND,
    ENUM_PDU_TYPE_MAX = 0xffffffff
};

enum ENUM_ADD_FRI_TYPE{
    ENUM_PERNAME_NOT_EXIST = -1,
    ENUM_PERNAME_NOT_ONLINE,
    ENUM_PERNAME_IS_ONLINE
};

struct FileInfo{
    char caFileName[32];
    int iFileType;
};
struct PDU{             //size:76
  uint uintPDULen;      //总的协议数据单元大小
  uint uintPDUType;     //消息类型

  char caData[64];

  uint uintMsgLen;      //实际消息的大小
  int Msg[];            //实际消息
};

struct PDU *mkPDU(uint uintMsgLen = 0);

//class SystemError{
//public:
//    enum ENUM_SYSTEM_ERROR{
//        ENUM_SYSTEM_ERROR_MIN = 0x00000000,
//        ENUM_SYSTEM_ERROR_PARAMETER,
//        ENUM_SYSTEM_ERROR_MAX = 0xffffffff
//    };
//};

#endif // PROTOCOL_H
