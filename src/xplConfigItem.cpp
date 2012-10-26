/***************************************************************************
****																	****
****	xplConfigItem.cpp												****
****																	****
****	Container for xpl device configuration data		 				****
****																	****
****	Copyright (c) 2005 Mal Lansell.									****
****    Email: xpl@lansell.org                                          ****
****																	****
****	Permission is hereby granted, free of charge, to any person		****
****	obtaining a copy of this software and associated documentation	****
****	files (the "Software"), to deal in the Software without			****
****	restriction, including without limitation the rights to use,	****
****	copy, modify, merge, publish, distribute, sublicense, and/or	****
****	sell copies of the Software, and to permit persons to whom the	****
****	Software is furnished to do so, subject to the following		****
****	conditions:														****
****																	****
****	The above copyright notice and this permission notice shall		****
****	be included in all copies or substantial portions of the		****
****	Software.														****
****																	****
****	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY		****
****	KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE		****
****	WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR			****
****	PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR	****
****	COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER		****
****	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR			****
****	OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE		****
****	SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.			****
****																	****
***************************************************************************/

#include "xplCore.h"
#include "xplConfigItem.h"
#include "xplStringUtils.h"

using namespace xpl;


/***************************************************************************
****																	****
****	xplConfigItem::AddValue											****
****																	****
***************************************************************************/

bool xplConfigItem::AddValue ( string const& _value )
{
    if ( GetNumValues() >= m_maxValues )
    {
        return false;
    }

    xplMsgItem::AddValue ( _value );
    return true;
}


/***************************************************************************
****																	****
****	xplConfigItem::RegistryLoad										****
****																	****
****	Read config values from the registry (Not PocketPC)				****
****																	****
***************************************************************************/
/*
bool xplConfigItem::RegistryLoad( HKEY& _hKey )
{
	// Remove the existing values
	ClearValues();

	// Get the size of the buffer required
	 bufferSize;
#ifdef UNICODE
	UnicodeString unicodeStr = UnicodeString( GetName().c_str() );
	LONG res = RegQueryValueEx( _hKey, unicodeStr.c_str(), NULL, NULL, NULL, &bufferSize );
#else
	LONG res = RegQueryValueEx( _hKey, GetName().c_str(), NULL, NULL, NULL, &bufferSize );
#endif

	if( ERROR_SUCCESS != res )
	{
		return false;
	}

	// Read the registry data into a buffer
	int8* pBuffer = new int8[bufferSize];
	 type;
#ifdef UNICODE
	res = RegQueryValueEx( _hKey, unicodeStr.c_str(), NULL, &type, (uint8*)pBuffer, &bufferSize );
	int8* pNewBuffer = new int8[bufferSize];
	WideCharToMultiByte( CP_ACP, 0, (WCHAR*)pBuffer, -1, pNewBuffer, bufferSize, NULL, NULL );
	delete [] pBuffer;
	pBuffer = pNewBuffer;
#else
	res = RegQueryValueEx( _hKey, GetName().c_str(), NULL, &type, (uint8*)pBuffer, &bufferSize );
#endif
	if( ( ERROR_SUCCESS != res ) || ( REG_MULTI_SZ != type ) )
	{
		return false;
	}

	// Read the values from the buffer
	int8* pPos = pBuffer;
	while( *pPos )
	{
		string value = string( (int8*)pPos );
		AddValue( value );

		pPos += (value.size()+1);
	}

	// Clean up
	delete [] pBuffer;
	return true;
}*/


/***************************************************************************
****																	****
****	xplConfigItem::RegistrySave										****
****																	****
****	Write config values to the registry (Not PocketPC)				****
****																	****
***************************************************************************/
/*
bool xplConfigItem::RegistrySave( HKEY& _hKey )const
{
	// Values must be concatenated into a single buffer

	// First calculate the size of buffer
	// required to hold all the values
	uint32 total = 0;
	uint32 i;
	for( i=0; i<GetNumValues(); ++i )
	{
		total += (uint32)(GetValue(i).size() + 1);
	}

	// Array is terminated by an empty string
	total += 1;

	// Now build the buffer
	int8* pBuffer = new int8[total];
	int8* pPos = pBuffer;

	for( i=0; i<GetNumValues(); ++i )
	{
		string const& value = GetValue(i);
		strcpy( pPos, value.c_str() );
		pPos += (value.size() + 1);
	}
	*pPos = 0;

	// Set the registry value
#ifdef UNICODE
	UnicodeString unicodeStr = UnicodeString( GetName().c_str() );
	WCHAR* pWideBuffer = new WCHAR[total];
	MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, pBuffer, total, pWideBuffer, total<<1 );
	RegSetValueEx( _hKey, unicodeStr.c_str(), 0, REG_MULTI_SZ, (uint8*)pWideBuffer, total<<1 );
	delete [] pWideBuffer;
#else
	RegSetValueEx( _hKey, GetName().c_str(), 0, REG_MULTI_SZ, (uint8*)pBuffer, total );
#endif

	// Clean up
	delete [] pBuffer;
	return true;
}
*/

/***************************************************************************
****																	****
****	xplConfigItem::FileLoad											****
****																	****
****	Read config values from a file (PocketPC Only)					****
****	Mimics the data format used in RegistryLoad()					****
****																	****
***************************************************************************/
/*
bool xplConfigItem::FileLoad( HANDLE& _hFile )
{
	// Remove the existing values
	ClearValues();

	// Get the size of the buffer required
	 bufferSize, bytesRead;
	ReadFile( _hFile, &bufferSize, sizeof(bufferSize), &bytesRead, NULL );

	// Read the config data into a buffer
	int8* pBuffer = new int8[bufferSize];
	ReadFile( _hFile, pBuffer, bufferSize, &bytesRead, NULL );

	// Read the values from the buffer
	int8* pPos = pBuffer;
	while( *pPos )
	{
		string value = string( pPos );
		AddValue( value );

		pPos += (value.size()+1);
	}

	// Clean up
	delete [] pBuffer;
	return true;
}*/


/***************************************************************************
****																	****
****	xplConfigItem::FileSave											****
****																	****
****	Write config values to a file (PocketPC Only)					****
****	Mimics the data format used in RegistrySave()					****
****																	****
***************************************************************************/
/*
bool xplConfigItem::FileSave( HANDLE& _hFile )const
{
	// Values must be concatenated into a single buffer

	// First calculate the size of buffer
	// required to hold all the values
	uint32 total = 0;
	uint32 i;
	for( i=0; i<GetNumValues(); ++i )
	{
		total += (uint32)(GetValue(i).size() + 1);
	}

	// Array is terminated by an empty string
	total += 1;

	// Now build the buffer
	int8* pBuffer = new int8[total];
	int8* pPos = pBuffer;

	for( i=0; i<GetNumValues(); ++i )
	{
		string const& value = GetValue(i);
		strcpy( pPos, value.c_str() );
		pPos += (value.size() + 1);
	}
	*pPos = 0;

	// Write the size of the buffer
	 bytesWritten;
	WriteFile( _hFile, &total, sizeof(total), &bytesWritten, NULL );

	// Write the buffer
	WriteFile( _hFile, pBuffer, total, &bytesWritten, NULL );

	// Clean up
	delete [] pBuffer;
	return true;
}*/


