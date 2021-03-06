//
//  BitmapAtlasGenerator.cpp
//  OpenSpades
//
//  Created by yvt on 7/28/13.
//  Copyright (c) 2013 yvt.jp. All rights reserved.
//

#include "BitmapAtlasGenerator.h"
#include "Bitmap.h"
#include "../binpack2d/binpack2d.hpp"
#include "Debug.h"
#include "Exception.h"
#include <string.h>

namespace spades {
	BitmapAtlasGenerator::BitmapAtlasGenerator() {
		
	}
	BitmapAtlasGenerator::~BitmapAtlasGenerator() {
		
	}
	void BitmapAtlasGenerator::AddBitmap(spades::Bitmap *b) {
		bmps.push_back(b);
	}
	
	int BitmapAtlasGenerator::MinRectSize() {
		int area = 0;
		for(size_t i = 0; i < bmps.size(); i++){
			Bitmap *bmp = bmps[i];
			area += bmp->GetWidth() * bmp->GetHeight();
		}
		
		for(int b = 1; b < 16; b++){
			int s = 1 << b;
			if(s * s >= area)
				return s;
		}
		SPAssert(false);
	}
	
	int BitmapAtlasGenerator::MinWidth() {
		int w = 0;
		for(size_t i = 0; i < bmps.size(); i++){
			Bitmap *bmp = bmps[i];
			if(bmp->GetWidth() > w) w = bmp->GetWidth();
		}
		
		for(int b = 1; b < 16; b++){
			int s = 1 << b;
			if(s >= w)
				return s;
		}
		SPAssert(false);
	}
	
	int BitmapAtlasGenerator::MinHeight() {
		int w = 0;
		for(size_t i = 0; i < bmps.size(); i++){
			Bitmap *bmp = bmps[i];
			if(bmp->GetHeight() > w) w = bmp->GetWidth();
		}
		
		for(int b = 1; b < 16; b++){
			int s = 1 << b;
			if(s >= w)
				return s;
		}
		SPAssert(false);
	}
	
	BitmapAtlasGenerator::Result BitmapAtlasGenerator::Pack() {
		BinPack2D::ContentAccumulator<Item> items;
		int minSize = MinRectSize();
		int minW = MinWidth(), minH = MinHeight();
		minSize = std::max(minSize, std::min(minW, minH));
		int mw = minSize, mh = minSize;
		
		for(size_t i = 0; i < bmps.size(); i++){
			Bitmap *b = bmps[i];
			Item itm = {b,0,0,b->GetWidth(),b->GetHeight()};
			items += BinPack2D::Content<Item>(itm,
											  BinPack2D::Coord(),
											  BinPack2D::Size(itm.w,itm.h),
											  false );
		}
		
		while(true){
			int ww = std::max(mw, minW), hh = std::max(mh, minH);
			BinPack2D::CanvasArray<Item> canvasArray =
			BinPack2D::UniformCanvasArrayBuilder<Item>(ww,hh,1).Build();
			
			BinPack2D::ContentAccumulator<Item> remainder;
			
			if(canvasArray.Place( items, remainder ) && remainder.Get().empty()){
				BinPack2D::ContentAccumulator<Item> output;
				canvasArray.CollectContent(output);
				
				Bitmap *outbmp = new Bitmap(ww, hh);
				uint32_t *outPixels = outbmp->GetPixels();

				Result result;
				for(BinPack2D::Content<Item>::Vector::iterator it =
					output.Get().begin();it != output.Get().end();it++){
					const BinPack2D::Content<Item>& c = *it;
					Item itm = c.content;
					itm.x = c.coord.x;
					itm.y = c.coord.y;
					result.items.push_back(itm);
					
					Bitmap *b = itm.bitmap;
					uint32_t *inPixels = b->GetPixels();
					int w = itm.w, h = itm.h;
					
					for(int i = 0; i < h; i++){
						memcpy(outPixels + itm.x + (itm.y + i) * ww,
							   inPixels + i * w,
							   w * 4);
					}
					
				}
				
				result.bitmap = outbmp;
				
				return result;
			}
			
			mw <<= 1; mh <<= 1;
		}
	}
}
