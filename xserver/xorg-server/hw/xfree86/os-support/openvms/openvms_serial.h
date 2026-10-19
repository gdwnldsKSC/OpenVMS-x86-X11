#ifndef OPENVMS_SERIAL_H
#define OPENVMS_SERIAL_H

/* Ordinary-context byte stream for the native, already initialized i8042. */
int OpenVMSSerialOwns(int token);
int OpenVMSSerialReady(void);
int OpenVMSSerialFeedByte(unsigned char value);
void OpenVMSSerialDrain(void);
void OpenVMSSerialReset(void);
struct _InputInfoRec;
void OpenVMSSerialAttach(struct _InputInfoRec *info);

#endif
