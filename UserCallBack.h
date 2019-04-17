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
	//业务层处理包回调，如果需要返回包，可以直接在respond参数和respondlen参数返回，长度不得超过40960
	virtual void OnPacketArrived(int fd,const char* packet,int packetlen,char* respond,int& respondlen) = 0;
};