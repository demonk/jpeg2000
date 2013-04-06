#ifndef BIT_IO_STREAM
#define BIT_IO_STREAM

namespace bitInputOutput{
	static unsigned char *bio_start;
	static unsigned char *bio_end;
	static unsigned char *bio_bp;

	static unsigned int bio_buf;
	static unsigned int bio_ct;


	static int getPosition();
	static void initEncoder(unsigned char *bp,int len);

	static void writeBit(int b);
	static void writeBits(int v,int n);
	static int writeByte();

	static int readBit();
	static int readBits(int n);
	static int readByte();

	static int flush();
	static int inAlign();

	void initEncoder(unsigned char *bp,int len)
	{
		bio_start=bp;
		bio_end=bp+len;
		bio_bp=bp;
		bio_buf=0;
		bio_ct=8;
	}
	int getPosition()
	{
		return bio_bp-bio_start;
	}
	int writeByte()
	{
		bio_buf=(bio_buf<<8)&0xffff;
		bio_ct=bio_buf==0xff00?7:8;
		if(bio_bp>=bio_end)
			return 1;
		*bio_bp++=bio_buf>>8;
		return 0;
	}
	void writeBit(int b)
	{
		if(bio_ct==0)
			writeByte();

		bio_ct--;
		bio_buf=bio_buf|b<<bio_ct;
	}
	void writeBits(int v,int n)
	{
		for(int i=n-1;i>=0;i--)
		{
			writeBit((v>>i)&1);
		}
	}
	int readBit()
	{
		if(bio_ct==0)
			readByte();

		bio_ct--;
		return (bio_buf>>bio_ct)&1;
	}
	int readByte()
	{
		bio_buf=(bio_buf<<8)&0xffff;
		bio_ct=bio_buf==0xff00?7:8;
		if(bio_bp>=bio_end)
			return 1;
		bio_buf=bio_buf|*bio_bp++;
		return 0;
	}
	int readBits(int n)
	{
		bio_buf=(bio_buf<<8)&0xffff;
		bio_ct=bio_buf==0xff00?7:8;
		if(bio_bp>=bio_end)
			return 1;

		bio_buf=bio_buf|*bio_bp++;
		return 0;
	}
	int flush()
	{
		bio_ct=0;
		if(writeByte())
			return 1;

		if(bio_ct==7)
		{
			bio_ct=0;

			if(writeByte())
			{
				return 1;
			}
		}
		return 0;
	}
}
#endif