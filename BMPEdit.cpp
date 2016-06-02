// BMPEdit.cpp : implementation file
//

#include "stdafx.h"
#include "02test.h"
#include "BMPEdit.h"
#include "math.h"
#include "iostream"
using namespace std;

/////add20150108
#define PI 3.14159


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBMPEdit

CBMPEdit::CBMPEdit()
{
}

CBMPEdit::~CBMPEdit()
{
}



/////////////////////////////////////////////////////////////////////////////
// CBMPEdit message handlers




BOOL CBMPEdit::LoadBmpFile(const CString &sBmpFileName)
{
	m_bReady = FALSE;

	//�ͷ�֮ǰ��ȡ������///////
	//�ͷ�ͼ������
	if(m_hgImageData)
		GlobalFree(m_hgImageData);
	
	m_sBMPFileFullName = sBmpFileName;
	if(m_sBMPFileFullName.IsEmpty())
		return FALSE;
	CFile BmpFile;
	if(!BmpFile.Open(m_sBMPFileFullName,CFile::modeRead))
	{
		AfxMessageBox("�ļ���ʱ����!");
		return FALSE;
	}
	//��ʽ���
	if(BmpFile.Read(&m_BmpFileHeader,sizeof(BITMAPFILEHEADER)) < sizeof(BITMAPFILEHEADER))
	{
		AfxMessageBox("�ļ���������!");
		BmpFile.Close();
		return FALSE;
	}
	if(m_BmpFileHeader.bfType != 0x4D42)
	{
		AfxMessageBox("��λͼ�ļ�");
		BmpFile.Close();
		return FALSE;
	}
	if(BmpFile.Read(&m_BmpInfo.bmiHeader,sizeof(BITMAPINFOHEADER)) < sizeof(BITMAPINFOHEADER))
	{
		AfxMessageBox("�ļ���������!");
		BmpFile.Close();
		return FALSE;
	}
	//SetDIBitsToDevice()��StretchDIBits()�����д����ɫ�壬����Ҫ����ǰDC��ѡ���ɫ���ˣ�����
	
	//��ȡ��ɫ��
	if(m_BmpInfo.bmiHeader.biBitCount < 16)
		BmpFile.Read(&m_BmpInfo.bmiColors,GetColorNum(m_BmpInfo.bmiHeader) * sizeof(RGBQUAD));
	
	//��ȡͼ������
	 nBytePerLine = (m_BmpInfo.bmiHeader.biWidth*m_BmpInfo.bmiHeader.biBitCount + 31)/32*4;//ͼ��ÿ����ռʵ���ֽ���������4�ı�����
	m_hgImageData = GlobalAlloc(GHND, nBytePerLine * m_BmpInfo.bmiHeader.biHeight);

	if(m_hgImageData == NULL)
	{
		AfxMessageBox("�ڴ治��");
		BmpFile.Close();
		return FALSE;
	}
	pImageData = (byte *)GlobalLock(m_hgImageData);
	BmpFile.Read(pImageData,nBytePerLine * m_BmpInfo.bmiHeader.biHeight);
	

	
	Gray();



	Sharpening();




	Binarization();



	GlobalUnlock(m_hgImageData);
	BmpFile.Close();
	m_bReady = TRUE;
	return TRUE;
}

int CBMPEdit::GetColorNum(const BITMAPINFOHEADER &BmpInf)
{
	if(BmpInf.biClrUsed >0)
		return BmpInf.biClrUsed;
	else
		return 1 << BmpInf.biBitCount;
}

BOOL CBMPEdit::PlayBmp(CDC *pDC, const CPoint &StartPoint)
{	
	if(!m_bReady)
		return FALSE;

	byte *pData = (byte *)GlobalLock(m_hgImageData);
	if(!SetDIBitsToDevice(pDC->GetSafeHdc(),
						StartPoint.x,StartPoint.y,
						m_BmpInfo.bmiHeader.biWidth,m_BmpInfo.bmiHeader.biHeight,
						0,0,
						0,m_BmpInfo.bmiHeader.biHeight,
						pData,
					//	pBmpInf,
					    (BITMAPINFO*)&m_BmpInfo,
						DIB_RGB_COLORS
						))
	 {
		 GlobalUnlock(m_hgImageData);
		 return FALSE;
	 }
	GlobalUnlock(m_hgImageData);


/////libin///20150118////start////////
	//byte *pData = (byte *)GlobalLock(hDIB);
	//if(!SetDIBitsToDevice(pDC->GetSafeHdc(),
	//	StartPoint.x,StartPoint.y,
	//	m_BmpInfo.bmiHeader.biWidth,m_BmpInfo.bmiHeader.biHeight,
	//	0,0,
	//	0,m_BmpInfo.bmiHeader.biHeight,
	//	pData,
	//	//	pBmpInf,
	//	(BITMAPINFO*)&m_BmpInfo,
	//	DIB_RGB_COLORS
	//	))
	//{
	//	GlobalUnlock(hDIB);
	//	return FALSE;
	//}
	//GlobalUnlock(hDIB);

//////////////end////////////////////////
	return TRUE;

}
void CBMPEdit::Gray()
{
	pImageData = (byte *)GlobalLock(m_hgImageData);
	lWidth = m_BmpInfo.bmiHeader.biWidth;
	lHeight = m_BmpInfo.bmiHeader.biHeight;

	//add your code here
	int sum=0;
	for(int i=0;i<lWidth*lHeight*3;i=i+3)
	{ 
		sum=*(pImageData+i)+*(pImageData+1+i)+*(pImageData+2+i);
		*(pImageData+i)=sum/3;
		*(pImageData+i+1)=sum/3;
		*(pImageData+i+2)=sum/3;
	}
	GlobalUnlock(m_hgImageData);
}

int CBMPEdit:: Sharpening()
{
	lWidth = m_BmpInfo.bmiHeader.biWidth;
	lHeight = m_BmpInfo.bmiHeader.biHeight;

	hDIB = GlobalAlloc(GHND, nBytePerLine * lHeight);

	if (hDIB == NULL)
	{

		return NULL;
	}
	//pImageDataNew = (byte *)GlobalLock(hDIB);

	int Gx[3][3]={-1,0,1,-2,0,2,-1,0,1};
	int Gy[3][3]={1,2,1,0,0,0,-1,-2,-1};
	int dx,dy,i1,j1,i2,j2 ,temp;
	for(int i=1;i<lWidth-1;i++)
		for(int j=1;j<lHeight-1;j++)
		{
			dx=0,dy=0;
			for(i1=i,i2=0;i2<3;++i1,++i2)
				for(j1=j,j2=0;j2<3;++j1,++j2)
				{
					int temp=*(pImageData + i1* m_BmpInfo.bmiHeader.biHeight*3+j1*3);
					dx+=Gx[i2][j2]*temp;
					dy+=Gy[i2][j2]*temp;
				}
				dx=(int)sqrt(float(dx*dx+dy*dy));
				for(int k=0;k<3;k++)
					*(pImageData + i* m_BmpInfo.bmiHeader.biHeight*3+j*3+k)=dx;
		}
	GlobalUnlock(hDIB);
}
void CBMPEdit::Binarization()
{
	pImageData = (byte *)GlobalLock(m_hgImageData);
	lWidth = m_BmpInfo.bmiHeader.biWidth;
	lHeight = m_BmpInfo.bmiHeader.biHeight;
	for(int i=0;i<lWidth*lHeight*3;i++)
	{
		if(*(pImageData+i)>128)
			*(pImageData+i)=255;
		else
			*(pImageData+i)=0;
	}
}