#ifndef XCF_H
#define XCF_H
/*
 * qxcfi.cpp: A Qt 3 plug-in for reading GIMP XCF image files
 * Copyright (C) 2001 lignum Computing, Inc. <allen@lignumcomputing.com>
 * Copyright (C) 2004 Melchior FRANZ <mfranz@kde.org>
 *
 * This plug-in is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */


#include <QImageIOHandler>
#include <QImage>
#include <QIODevice>
#include <QVector>

#include "gimp.h"

class XCFHandler : public QImageIOHandler
{
public:
    XCFHandler();

    bool canRead() const;
    bool read(QImage *image);
    bool write(const QImage &image);

    QByteArray name() const;

    static bool canRead(QIODevice *device);
};

const float INCHESPERMETER = (100.0 / 2.54);

/*!
 * Each layer in an XCF file is stored as a matrix of
 * 64-pixel by 64-pixel images. The GIMP has a sophisticated
 * method of handling very large images as well as implementing
 * parallel processing on a tile-by-tile basis. Here, though,
 * we just read them in en-masse and store them in a matrix.
 */
typedef QVector<QVector<QImage> > Tiles;



class XCFImageFormat {
public:
	XCFImageFormat();
	bool readXCF(QIODevice *device, QImage *image);


private:
	/*!
	 * Each GIMP image is composed of one or more layers. A layer can
	 * be one of any three basic types: RGB, grayscale or indexed. With an
	 * optional alpha channel, there are six possible types altogether.
	 *
	 * Note: there is only ever one instance of this structure. The
	 * layer info is discarded after it is merged into the final QImage.
	 */
	class Layer {
	public:
		quint32 width;			//!< Width of the layer
		quint32 height;		//!< Height of the layer
		qint32 type;			//!< Type of the layer (GimpImageType)
		char* name;			//!< Name of the layer
		quint32 hierarchy_offset;	//!< File position of Tile hierarchy
		quint32 mask_offset;		//!< File position of mask image

		uint nrows;			//!< Number of rows of tiles (y direction)
		uint ncols;			//!< Number of columns of tiles (x direction)

		Tiles image_tiles;		//!< The basic image
		//! For Grayscale and Indexed images, the alpha channel is stored
		//! separately (in this data structure, anyway).
		Tiles alpha_tiles;
		Tiles mask_tiles;		//!< The layer mask (optional)

		//! Additional information about a layer mask.
		struct {
			quint32 opacity;
			quint32 visible;
			quint32 show_masked;
			uchar red, green, blue;
			quint32 tattoo;
		} mask_channel;

		bool active;			//!< Is this layer the active layer?
		quint32 opacity;		//!< The opacity of the layer
		quint32 visible;		//!< Is the layer visible?
		quint32 linked;		//!< Is this layer linked (geometrically)
		quint32 preserve_transparency; //!< Preserve alpha when drawing on layer?
		quint32 apply_mask;		//!< Apply the layer mask?
		quint32 edit_mask;		//!< Is the layer mask the being edited?
		quint32 show_mask;		//!< Show the layer mask rather than the image?
		qint32 x_offset;		//!< x offset of the layer relative to the image
		qint32 y_offset;		//!< y offset of the layer relative to the image
		quint32 mode;			//!< Combining mode of layer (LayerModeEffects)
		quint32 tattoo;		//!< (unique identifier?)

		//! As each tile is read from the file, it is buffered here.
		uchar tile[TILE_WIDTH * TILE_HEIGHT * sizeof(QRgb)];

		//! The data from tile buffer is copied to the Tile by this
		//! method.  Depending on the type of the tile (RGB, Grayscale,
		//! Indexed) and use (image or mask), the bytes in the buffer are
		//! copied in different ways.
		void (*assignBytes)(Layer& layer, uint i, uint j);

		Layer(void) : name(0) {}
		~Layer(void) { delete[] name; }
	};


	/*!
	 * The in-memory representation of the XCF Image. It contains a few
	 * metadata items, but is mostly a container for the layer information.
	 */
	class XCFImage {
	public:
		quint32 width;			//!< width of the XCF image
		quint32 height;		//!< height of the XCF image
		qint32 type;			//!< type of the XCF image (GimpImageBaseType)

		quint8 compression;		//!< tile compression method (CompressionType)
		float x_resolution;		//!< x resolution in dots per inch
		float y_resolution;		//!< y resolution in dots per inch
		qint32 tattoo;			//!< (unique identifier?)
		quint32 unit;			//!< Units of The GIMP (inch, mm, pica, etc...)
		qint32 num_colors;		//!< number of colors in an indexed image
		QVector<QRgb> palette;	        //!< indexed image color palette

		int num_layers;			//!< number of layers
		Layer layer;			//!< most recently read layer

		bool initialized;		//!< Is the QImage initialized?
		QImage image;			//!< final QImage

		XCFImage(void) : initialized(false) {}
	};


	//! In layer DISSOLVE mode, a random number is chosen to compare to a
	//! pixel's alpha. If the alpha is greater than the random number, the
	//! pixel is drawn. This table merely contains the random number seeds
	//! for each ROW of an image. Therefore, the random numbers chosen
	//! are consistent from run to run.
	static int random_table[RANDOM_TABLE_SIZE];

	//! This table provides the add_pixel saturation values (i.e. 250 + 250 = 255).
	//static int add_lut[256][256]; - this is so lame waste of 256k of memory
	static int add_lut( int, int );

	//! The bottom-most layer is copied into the final QImage by this
	//! routine.
	typedef void (*PixelCopyOperation)(Layer& layer, uint i, uint j, int k, int l,
			QImage& image, int m, int n);

	//! Higher layers are merged into the the final QImage by this routine.
	typedef void (*PixelMergeOperation)(Layer& layer, uint i, uint j, int k, int l,
			QImage& image, int m, int n);

	//! Layer mode static data.
	typedef struct {
		bool affect_alpha;		//!< Does this mode affect the source alpha?
	} LayerModes;

	//! Array of layer mode structures for the modes described by
	//! LayerModeEffects.
	static const LayerModes layer_modes[];

	bool loadImageProperties(QDataStream& xcf_io, XCFImage& image);
	bool loadProperty(QDataStream& xcf_io, PropType& type, QByteArray& bytes);
	bool loadLayer(QDataStream& xcf_io, XCFImage& xcf_image);
	bool loadLayerProperties(QDataStream& xcf_io, Layer& layer);
	bool composeTiles(XCFImage& xcf_image);
	void setGrayPalette(QImage& image);
	void setPalette(XCFImage& xcf_image, QImage& image);
	static void assignImageBytes(Layer& layer, uint i, uint j);
	bool loadHierarchy(QDataStream& xcf_io, Layer& layer);
	bool loadLevel(QDataStream& xcf_io, Layer& layer, qint32 bpp);
	static void assignMaskBytes(Layer& layer, uint i, uint j);
	bool loadMask(QDataStream& xcf_io, Layer& layer);
	bool loadChannelProperties(QDataStream& xcf_io, Layer& layer);
	bool initializeImage(XCFImage& xcf_image);
	bool loadTileRLE(QDataStream& xcf_io, uchar* tile, int size,
			int data_length, qint32 bpp);
	static void copyLayerToImage(XCFImage& xcf_image);
	static void copyRGBToRGB(Layer& layer, uint i, uint j, int k, int l,
			QImage& image, int m, int n);

	static void copyGrayToGray(Layer& layer, uint i, uint j, int k, int l,
			QImage& image, int m, int n);
	static void copyGrayToRGB(Layer& layer, uint i, uint j, int k, int l,
			QImage& image, int m, int n);
	static void copyGrayAToRGB(Layer& layer, uint i, uint j, int k, int l,
			QImage& image, int m, int n);
	static void copyIndexedToIndexed(Layer& layer, uint i, uint j, int k, int l,
			QImage& image, int m, int n);
	static void copyIndexedAToIndexed(Layer& layer, uint i, uint j, int k, int l,
			QImage& image, int m, int n);
	static void copyIndexedAToRGB(Layer& layer, uint i, uint j, int k, int l,
			QImage& image, int m, int n);

	static void mergeLayerIntoImage(XCFImage& xcf_image);
	static void mergeRGBToRGB(Layer& layer, uint i, uint j, int k, int l,
			QImage& image, int m, int n);
	static void mergeGrayToGray(Layer& layer, uint i, uint j, int k, int l,
			QImage& image, int m, int n);
	static void mergeGrayAToGray(Layer& layer, uint i, uint j, int k, int l,
			QImage& image, int m, int n);
	static void mergeGrayToRGB(Layer& layer, uint i, uint j, int k, int l,
			QImage& image, int m, int n);
	static void mergeGrayAToRGB(Layer& layer, uint i, uint j, int k, int l,
			QImage& image, int m, int n);
	static void mergeIndexedToIndexed(Layer& layer, uint i, uint j, int k, int l,
			QImage& image, int m, int n);
	static void mergeIndexedAToIndexed(Layer& layer, uint i, uint j, int k, int l,
			QImage& image, int m, int n);
	static void mergeIndexedAToRGB(Layer& layer, uint i, uint j, int k, int l,
			QImage& image, int m, int n);

	static void dissolveRGBPixels(QImage& image, int x, int y);
	static void dissolveAlphaPixels(QImage& image, int x, int y);
};

#endif
