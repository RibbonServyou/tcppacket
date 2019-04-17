#pragma once

/** @file UserCallBack.h
*  @note Hangzhou Hikvision System Technology Co., Ltd. All Rights Reserved.
*  @brief 用户的事件通知。
*  //使用说明
*  @author xuhuajie
*  @date 2017/08/21
*
*  @note 历史记录：
*  @note V1.0.0.0 创建
*/

class ITcpPacketNotify
{
public:
	virtual void OnConnected(int fd) = 0;
	virtual void OnDisConnected(int fd) = 0;
	virtual void OnTimeOutError(int fd) = 0;
	virtual void OnError(int fd,int errcode,const char**msg ) = 0;
	//提取包，需要业务层返回解析出包的长度，或者舍弃一些不合格的包，成功解析出包返回true
	virtual bool OnAnalyzePacket(int fd,const char* buff,int bufflen,int& packetlen,int &ignore) = 0;
	//业务层处理包回调
        //最新版本中函数内返回已废弃，即respond，respondlen未使用。原因是节省内存，业务层可以使用SendPacket接口返回。
        //节省内存后，单台服务器可以开到3000的连接。如30*100的配置。
	virtual void OnPacketArrived(int fd,const char* packet,int packetlen,char* respond,int& respondlen) = 0;
};