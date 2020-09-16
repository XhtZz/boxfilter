//////第一版 暴力版本(复杂度:width*height*(2*radius+1)*(2*radius+1))
void boxfilter(uchar* image, uchar* dst, int width, int height, int radius){
    for(int h=0;h<height;++h){
        int h_shift = h*weight;
        int start_h = max(0,height-radius);
        int end_h = min(h+radius,height-1);
        for(int w=0;w<width;++w){
            int start_w = max(0,w-radius);
            int end_w = max(w+radius,weight-1);

            int sum = 0;
            for(int i=start_h;i<end_h;++i){
                for(int j=start_w;j<end_w;++j){
                    sum += image[i*weight+j];
                }
            }
            image[h_shift+w] = sum;
        }   
    }
}

//////第二版 行列拆分(复杂度：wigth*height*(2*radius+1)*2)
void boxfilter(uchar* image, uchar* dst, int width, int height, int radius){
    int* temp = (int*)malloc(height*width*sizeof(int));
    //计算每一行
    for(int h=0;h<height;++h){
        int h_shift = h*weight;
        for(int w=0;w<weight;++w){
            int start_w = min(0,w-radius);
            int end_w = max(w+radius,weight-1)

            float sum = 0;
            for(int i=start_w;i<end_w;++i){
                sum += image[h_shift+i];
            }
            temp[h_shift+w] = sum;
        }
    }

    //计算每一列
    for(int h=0;h<height;++h){
        int h_shift = h*weight;
        int start_h = min(0,h-radius);
        int end_h = max(h+radius,height-1);
        for(int w=0;w<weight;++w){
            float sum = 0;
            for(int j=start_h;j<end_h;++j){
                sum += temp[j*weight+w];
            }
            dst[h_shift+w] = sum;
        }
    }
}

//////第三版 考虑复杂度不受radius大小的方法，很简单，计算下一个目标点，只要加上后一列减去前一列就可以，重复利用了中间数据的和（复杂度：weight*height*(2*2))
void boxfilter(uchar* image, uchar* dst, int width, int height, int radius){
    //水平方向
    int* temp = (int*)malloc(sizeof(int)*width*height);
    for(int h=0;h<height;++h){
        int shift_h = h*weight;
        int sum = 0;
        //head
        for(int i=0;i<radius;++i){
            sum += image[shift_h+i];
        }
        for(int i=0;i<=radius;++i){
            sum += image[shift_h+i+radius];
            temp[shift_h+i] = sum;
        }
        //middle
        for(int i=radius+1;i<width-radius;++i){
            sum += image[shift_h+i+radius];
            sum -= image[shift_h+i-radius-1];
            temp[shift_h+i] = sum;
        }
        //tail
        for(int i=width-radius;i<width;++i){
            sum -= image[shift_h+i-radius-1];
            temp[shift_h+i] = sum;
        }
    }

    //垂直方向
    int* row_temp = (int*)malloc(sizeof(int)*width);
    memset(row_temp,0,sizeof(int)*width);
    //head
    for(int h=0;h<radius;++h){
        int shift_h = h*width;
        for(int w=0;w<width;++w){
            row_temp[w] += temp[shift_h+w];
        }
    }
    for(int h=0;h<=radius;++h){
        int shift_h = h*width;
        for(int w=0;w<width;++w){
            row_temp[w] += temp[(h+radius)*width+w]
            dst[shift_h+w] = row_temp[w];
        }
    }
    //middle
    for(int h=radius+1;h<height-radius;++h){
        int shift_h = h*width;
        for(int w=0;w<width;++w){
            row_temp[w] += temp[(h+radius)*width+w];
            row_temp[w] -= temp[(h-radius-1)*width+w];
            dst[shift_h+w] = row_temp[w];
        }
    }
    //tail
    for(int h=height-radius;h<height;++h){
        int shift_h = h*width;
        for(int w=0;w<width;++w){
            row_temp[w] -= temp[(h-radius-1)*width+w];
            dst[shift_h+w] = row_temp[w];
        }
    }
    free(row_temp);
    free(temp);
}

////// 第四版 垂直方向利用neon intrinsic指令进行加速(只做了head部分，其他的类似)
void boxfilter(uchar* image, uchar* dst, int width, int height, int radius){
    //水平方向
    int* temp = (int*)malloc(sizeof(int)*width*height);
    for(int h=0;h<height;++h){
        int shift_h = h*weight;
        int sum = 0;
        //head
        for(int i=0;i<radius;++i){
            sum += image[shift_h+i];
        }
        for(int i=0;i<=radius;++i){
            sum += image[shift_h+i+radius];
            temp[shift_h+i] = sum;
        }
        //middle
        for(int i=radius+1;i<width-radius;++i){
            sum += image[shift_h+i+radius];
            sum -= image[shift_h+i-radius-1];
            temp[shift_h+i] = sum;
        }
        //tail
        for(int i=width-radius;i<width;++i){
            sum -= image[shift_h+i-radius-1];
            temp[shift_h+i] = sum;
        }
    }

    //垂直方向
    ushort* row_temp = (ushort*)malloc(sizeof(ushort)*width);
    memset(row_temp,0,sizeof(ushort)*width);
    int remain = width%16;
    //head
    for(int h=0;h<radius;++h){
        int shift_h = h*width;
        int w;
        for(w=0;w<width;w+=16){
            ushort* t = row_temp;
            uint8x16 temp_vector = vld1q_u8(temp+shift_h+w);

            uint16x8_t row_temp_vector = vld1q_u16(row_temp);
            row_temp += 8;
            row_temp_vector = vaddw_u8(row_temp_vector,vget_low_u8(temp_vector));
            vst1q_u16(t,row_temp_vector);

            row_temp_vector= vld1q_u16(row_temp);
            row_temp += 8;
            row_temp_vector = vaddw_u8(row_temp_vector,vget_high_u8(temp_vector));
            vst1q_u16(t+8,row_temp_vector);
        }
        for(;w<width;++w){
            row_temp[w] += temp[shift_h+w];
        }
    }
    for(int h=0;h<=radius;++h){
        int shift_h = h*width;
        for(int w=0;w<width;++w){
            row_temp[w] += temp[(h+radius)*width+w]
            dst[shift_h+w] = row_temp[w];
        }
    }
    //middle
    for(int h=radius+1;h<height-radius;++h){
        int shift_h = h*width;
        for(int w=0;w<width;++w){
            row_temp[w] += temp[(h+radius)*width+w];
            row_temp[w] -= temp[(h-radius-1)*width+w];
            dst[shift_h+w] = row_temp[w];
        }
    }
    //tail
    for(int h=height-radius;h<height;++h){
        int shift_h = h*width;
        for(int w=0;w<width;++w){
            row_temp[w] -= temp[(h-radius-1)*width+w];
            dst[shift_h+w] = row_temp[w];
        }
    }
    free(row_temp);
    free(temp);
}
