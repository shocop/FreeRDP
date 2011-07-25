/**
 * FreeRDP: A Remote Desktop Protocol Client
 * Update Data PDUs
 *
 * Copyright 2011 Marc-Andre Moreau <marcandre.moreau@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "update.h"

uint8 UPDATE_TYPE_STRINGS[][32] =
{
	"Orders",
	"Bitmap",
	"Palette",
	"Synchronize"
};

void rdp_recv_orders_update(rdpRdp* rdp, STREAM* s)
{
	uint16 numberOrders;

	stream_seek_uint16(s); /* pad2OctetsA (2 bytes) */
	stream_read_uint16(s, numberOrders); /* numberOrders (2 bytes) */
	stream_seek_uint16(s); /* pad2OctetsB (2 bytes) */
}

void rdp_read_bitmap_data(STREAM* s, BITMAP_DATA* bitmap_data)
{
	uint8* srcData;
	uint16 dstSize;
	uint16 bytesPerPixel;

	stream_read_uint16(s, bitmap_data->left);
	stream_read_uint16(s, bitmap_data->top);
	stream_read_uint16(s, bitmap_data->right);
	stream_read_uint16(s, bitmap_data->bottom);
	stream_read_uint16(s, bitmap_data->width);
	stream_read_uint16(s, bitmap_data->height);
	stream_read_uint16(s, bitmap_data->bpp);
	stream_read_uint16(s, bitmap_data->flags);
	stream_read_uint16(s, bitmap_data->length);
	stream_get_mark(s, srcData);
	stream_seek(s, bitmap_data->length);

	bytesPerPixel = (bitmap_data->bpp + 7) / 8;
	dstSize = bitmap_data->width * bitmap_data->height * bytesPerPixel;

	if (bitmap_data->data != NULL)
		bitmap_data->data = (uint8*) xrealloc(bitmap_data->data, dstSize);
	else
		bitmap_data->data = (uint8*) xmalloc(dstSize);

	/*printf("width:%d height:%d bitsPerPixel:%d bytesPerPixel:%d dstSize:%d\n",
			bitmap_data->width, bitmap_data->height, bitmap_data->bpp, bytesPerPixel, dstSize);*/
}

void rdp_recv_bitmap_update(rdpRdp* rdp, STREAM* s)
{
	uint16 numberRectangles;
	BITMAP_DATA bitmap_data;

	bitmap_data.data = NULL;
	stream_read_uint16(s, numberRectangles); /* numberRectangles (2 bytes) */

	/* rectangles */
	while (numberRectangles > 0)
	{
		rdp_read_bitmap_data(s, &bitmap_data);
		numberRectangles--;
	}

	if (bitmap_data.data != NULL)
		xfree(bitmap_data.data);
}

void rdp_recv_palette_update(rdpRdp* rdp, STREAM* s)
{
	stream_seek_uint16(s); /* pad2Octets (2 bytes) */
	stream_seek_uint32(s); /* numberColors (4 bytes), must be set to 256 */

	/* paletteEntries */
}

void rdp_recv_synchronize_update(rdpRdp* rdp, STREAM* s)
{
	stream_seek_uint16(s); /* pad2Octets (2 bytes) */

	/**
	 * The Synchronize Update is an artifact from the
	 * T.128 protocol and should be ignored.
	 */
}

void rdp_recv_update_data_pdu(rdpRdp* rdp, STREAM* s)
{
	uint16 updateType;

	stream_read_uint16(s, updateType); /* updateType (2 bytes) */

	printf("%s Update Data PDU\n", UPDATE_TYPE_STRINGS[updateType]);

	switch (updateType)
	{
		case UPDATE_TYPE_ORDERS:
			rdp_recv_orders_update(rdp, s);
			break;

		case UPDATE_TYPE_BITMAP:
			rdp_recv_bitmap_update(rdp, s);
			break;

		case UPDATE_TYPE_PALETTE:
			rdp_recv_palette_update(rdp, s);
			break;

		case UPDATE_TYPE_SYNCHRONIZE:
			rdp_recv_synchronize_update(rdp, s);
			break;
	}
}
