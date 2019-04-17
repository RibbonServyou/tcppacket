#pragma once

/** @file UserCallBack.h
*  @note Hangzhou Hikvision System Technology Co., Ltd. All Rights Reserved.
*  @brief �û����¼�֪ͨ��
*  //ʹ��˵��
*  @author xuhuajie
*  @date 2017/08/21
*
*  @note ��ʷ��¼��
*  @note V1.0.0.0 ����
*/

class ITcpPacketNotify
{
public:
	virtual void OnConnected(int fd) = 0;
	virtual void OnDisConnected(int fd) = 0;
	virtual void OnTimeOutError(int fd) = 0;
	virtual void OnError(int fd,int errcode,const char**msg ) = 0;
	//��ȡ������Ҫҵ��㷵�ؽ��������ĳ��ȣ���������һЩ���ϸ�İ����ɹ�������������true
	virtual bool OnAnalyzePacket(int fd,const char* buff,int bufflen,int& packetlen,int &ignore) = 0;
	//ҵ��㴦����ص��������Ҫ���ذ�������ֱ����respond������respondlen�������أ����Ȳ��ó���40960
	virtual void OnPacketArrived(int fd,const char* packet,int packetlen,char* respond,int& respondlen) = 0;
};