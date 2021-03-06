//
//  IStream.cpp
//  OpenSpades
//
//  Created by yvt on 7/12/13.
//  Copyright (c) 2013 yvt.jp. All rights reserved.
//

#include "IStream.h"
#include "Exception.h"
#include <algorithm>
#include "Debug.h"
#include "../Core/Debug.h"

namespace  spades {
	IStream::~IStream() {
	
	}
	int IStream::ReadByte() {
		SPNotImplemented();
	}
	
	size_t IStream::Read(void *out, size_t bytes) {
		SPADES_MARK_FUNCTION();
		
		char *buf = reinterpret_cast<char *>(out);
		size_t read = 0;
		while (bytes--) {
			int b = ReadByte();
			if(b == -1)
				break;
			*(buf++) = (char)b;
			read++;
		}
		return read;
	}
	
	std::string IStream::Read(size_t maxBytes) {
		SPADES_MARK_FUNCTION();
		
		std::string str;
		char buf[2048];
		size_t readBytes;
		while((readBytes = Read(buf, std::min((size_t)2048, maxBytes))) > 0){
			str.append(buf, readBytes);
			maxBytes -= readBytes;
		}
		return str;
	}
	
	std::string IStream::ReadAllBytes() {
		SPADES_MARK_FUNCTION();
		
		return Read(0x7fffffffUL);
	}
	
	void IStream::WriteByte(int){
		SPADES_MARK_FUNCTION();
		
		SPNotImplemented();
	}
	
	void IStream::Write(const void *inp, size_t bytes) {
		SPADES_MARK_FUNCTION();
		
		const unsigned char *buf = reinterpret_cast<const unsigned char *>(inp);
		while(bytes--){
			WriteByte((int)*(buf++));
		}
	}
	
	void IStream::Write(const std::string &str){
		SPADES_MARK_FUNCTION();
		
		Write(str.data(), str.size());
	}
	
	uint64_t IStream::GetPosition() {
		SPNotImplemented();
	}
	
	void IStream::SetPosition(uint64_t pos) {
		SPNotImplemented();
	}
	
	uint64_t IStream::GetLength() {
		SPNotImplemented();
	}
	
	void IStream::SetLength(uint64_t) {
		SPNotImplemented();
	}
	
	uint16_t IStream::ReadLittleShort() {
		SPADES_MARK_FUNCTION();
		
		// TODO: big endian support
		uint16_t data;
		if(Read(&data, 2) < 2)
			SPRaise("Failed to read 2 bytes");
		return data;
	}
	
	uint32_t IStream::ReadLittleInt() {
		SPADES_MARK_FUNCTION();
		
		// TODO: big endian support
		uint32_t data;
		if(Read(&data, 4) < 4)
			SPRaise("Failed to read 4 bytes");
		return data;
	}
	
	StreamHandle::StreamHandle():
	o(NULL){
		
	}
	
	StreamHandle::StreamHandle(IStream *stream){
		SPADES_MARK_FUNCTION();
		if(!stream)
			SPInvalidArgument("stream");
		o = new SharedStream(stream);
	}
	
	StreamHandle::StreamHandle(const StreamHandle& handle):
	o(handle.o){
		SPADES_MARK_FUNCTION_DEBUG();
		o->Retain();
	}
	
	StreamHandle::~StreamHandle(){
		SPADES_MARK_FUNCTION();
		Reset();
	}
	
	void StreamHandle::operator=(const spades::StreamHandle &h){
		if(o == h.o)
			return;
		
		SPADES_MARK_FUNCTION();
		SharedStream *old = o;
		o = h.o;
		o->Retain();
		old->Release();
	}
	
	IStream *StreamHandle::operator->() const{
		SPAssert(o);
		return o->stream;
	}
	
	StreamHandle::operator class spades::IStream *() const{
		SPAssert(o);
		return o->stream;
	}
	
	void StreamHandle::Reset() {
		if(o){
			o->Release();
			o = NULL;
		}
	}
	
	StreamHandle::SharedStream::SharedStream(IStream *s):
	stream(s), refCount(1){}
	
	StreamHandle::SharedStream::~SharedStream(){
		delete stream;
	}
	
	void StreamHandle::SharedStream::Retain(){
		refCount++;
	}
	
	void StreamHandle::SharedStream::Release(){
		SPAssert(refCount > 0);
		refCount--;
		if(refCount == 0)
			delete this;
	}
	
	
	
}
