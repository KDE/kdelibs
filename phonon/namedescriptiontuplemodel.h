/*  This file is part of the KDE project
    Copyright (C) 2006 Matthias Kretz <kretz@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#ifndef PHONON_NAMEDESCRIPTIONTUPLEMODEL_H
#define PHONON_NAMEDESCRIPTIONTUPLEMODEL_H

#include <QAbstractListModel>
#include <kdelibs_export.h>

template<class t> class QList;

namespace Phonon
{
	class NameDescriptionTuple;
	class AudioOutputDevice;
	class AudioCaptureDevice;
	class VideoCaptureDevice;
	class NameDescriptionTupleModelPrivate;

	/**
	 * \short The NameDescriptionTupleModel class provides a model from
	 * a list of NameDescriptionTuple.
	 *
	 * NameDescriptionTupleModel is a readonly model that supplies a list
	 * using NameDescriptionTuple::name for the text and
	 * NameDescriptionTuple::description for the tooltip. It also provides
	 * the methods moveUp and moveDown to order the list. The resulting
	 * order of the NameDescriptionTuple::index values can then be retrieved
	 * using tupleIndexOrder.
	 *
	 * An example use case would be to give the user a QComboBox to select
	 * the output device:
	 * \code
	 * QComboBox* cb = new QComboBox( parentWidget );
	 * NameDescriptionTupleModel* model = new NameDescriptionTupleModel( cb );
	 * model->setModelData( BackendCapabilities::availableAudioOutputDevices() );
	 * cb->setModel( model );
	 * cb->setCurrentIndex( 0 ); // select first entry
	 * \endcode
	 *
	 * And to retrieve the selected AudioOutputDevice:
	 * \code
	 * int cbIndex = cb->currentIndex();
	 * int tupleIndex = model->tupleIndexAtPositionIndex( cbIndex );
	 * AudioOutputDevice selectedDevice = AudioOutputDevice::fromIndex( tupleIndex );
	 * \endcode
	 *
	 * \author Matthias Kretz <kretz@kde.org>
	 */
	class PHONONCORE_EXPORT NameDescriptionTupleModel : public QAbstractListModel
	{
		Q_OBJECT
		Q_DECLARE_PRIVATE( NameDescriptionTupleModel )
		public:
			/**
			 * Constructs a NameDescriptionTuple model with the
			 * given \p parent.
			 */
			NameDescriptionTupleModel( QObject* parent = 0 );

			/**
			 * Sets the model data using the list provided by \p data.
			 *
			 * All previous model data is cleared.
			 */
			void setModelData( const QList<NameDescriptionTuple>& data );

			/**
			 * Sets the model data using the list provided by \p data.
			 *
			 * All previous model data is cleared.
			 *
			 * Same as above, provided for convenience that you don't have to
			 * convert a QList<AudioOutputDevice> to QList<NameDescriptionTuple>
			 * yourself.
			 */
			void setModelData( const QList<AudioOutputDevice>& data );

			/**
			 * Sets the model data using the list provided by \p data.
			 *
			 * All previous model data is cleared.
			 *
			 * Same as above, provided for convenience that you don't have to
			 * convert a QList<AudioCaptureDevice> to QList<NameDescriptionTuple>
			 * yourself.
			 */
			void setModelData( const QList<AudioCaptureDevice>& data );

			/**
			 * Sets the model data using the list provided by \p data.
			 *
			 * All previous model data is cleared.
			 *
			 * Same as above, provided for convenience that you don't have to
			 * convert a QList<VideoCaptureDevice> to QList<NameDescriptionTuple>
			 * yourself.
			 */
			void setModelData( const QList<VideoCaptureDevice>& data );

			/**
			 * Returns the number of rows in the model. This value corresponds
			 * to the size of the list passed through setModelData.
			 *
			 * \param parent The optional \p parent argument is used in most models to specify
			 * the parent of the rows to be counted. Because this is a list if a
			 * valid parent is specified the result will always be 0.
			 *
			 * Reimplemented from QAbstractItemModel.
			 *
			 * \see QAbstractItemModel::rowCount
			 */
			int rowCount( const QModelIndex& parent = QModelIndex() ) const;

			/**
			 * Returns data from the item with the given \p index for the specified
			 * \p role.
			 * If the view requests an invalid index, an invalid variant is
			 * returned.
			 *
			 * Reimplemented from QAbstractItemModel.
			 *
			 * \see QAbstractItemModel::data
			 * \see Qt::ItemDataRole
			 */
			QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const;

			/**
			 * Moves the item at the given \p index up. In the resulting list
			 * the items at index.row() and index.row() - 1 are swapped.
			 *
			 * Connected views are updated automatically.
			 */
			void moveUp( const QModelIndex& index );

			/**
			 * Moves the item at the given \p index down. In the resulting list
			 * the items at index.row() and index.row() + 1 are swapped.
			 *
			 * Connected views are updated automatically.
			 */
			void moveDown( const QModelIndex& index );

			/**
			 * Returns a list of indexes in the same order as they are in the
			 * model. The indexes come from the NameDescriptionTuple::index
			 * method.
			 *
			 * This is usefull to let the user define a list of preference.
			 */
			QList<int> tupleIndexOrder() const;

			/**
			 * Returns the NameDescriptionTuple::index for the tuple
			 * at the given position \p positionIndex. For example a
			 * QComboBox will give you the currentIndex as the
			 * position in the list. But to select the according
			 * AudioOutputDevice using AudioOutputDevice::fromIndex
			 * you can use this method.
			 *
			 * \param positionIndex The position in the list.
			 */
			int tupleIndexAtPositionIndex( int positionIndex ) const;

		protected:
			NameDescriptionTupleModel( NameDescriptionTupleModelPrivate& d, QObject* parent = 0 );
			NameDescriptionTupleModelPrivate* d_ptr;
	};
}

#endif // PHONON_NAMEDESCRIPTIONTUPLEMODEL_H
// vim: sw=4 ts=4 noet tw=80
