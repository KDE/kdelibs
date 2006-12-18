/* 
 *
 * $Id: sourceheader 511311 2006-02-19 14:51:05Z trueg $
 *
 * This file is part of the Nepomuk KDE project.
 * Copyright (C) 2006 Sebastian Trueg <trueg@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING" for the exact licensing terms.
 */

/**
 * \mainpage The NEPOMUK-KDE Meta Data library (KMetaData)
 *
 * This is the documentation of the NEPOMUK-KDE Meta Data library.
 *
 *
 * \section overview The General Idea
 *
 * Three types of meta data can be identified:
 *
 * -# Meta data that is stored with the data itself and is available at all
 * times. This includes id3 tags, the number of pages in a pdf document, or
 * even the size of a file or the subject of an email.
 * -# Meta data that is created by the user manually like annotations or tags 
 * that are assigned to files, emails, or whatever resources.
 * -# Meta data that can be gathered automatically by applications such as the
 * source of a downloaded file or the email an attachment was saved from or the
 * original when copying a file locally.
 *
 * Type 1 is already handled in many implementations. KDE itself includes the
 * KMetaFileInfo classes that allow extracting this kind of meta information from
 * files.
 *
 * KMetaData is intended for meta data of type 2 and 3. It provides an easy way to 
 * create and read meta data for arbitrary resources (this includes for example files 
 * or emails, but also contacts or maybe even a paragraph in a pdf file).
 *
 * The simplest type of meta data that can be handled with KMetaData is a comment. It
 * is a simple string associated with a resource (a file for example). This comment 
 * is created by the user using an application that is based on KMetaData.
 *
 * KMetaData's core is designed to allow arbitrary types of meta data, i.e. any resource
 * can be related with any other resource or value by simply naming the relation and
 * providing the value.
 * The power of KMetaData, however, lies in that it provides a class for each type
 * of resource. Each of these classes provide convinience methods to allow a simple
 * handling of the meta data. For an example have a look at the File class.
 *
 * The types of resources and their properties are defined in the \ref page_ontology.
 *
 * KMetaData is resource based. Thus, working with KMetaData is always done with instances
 * representing a certain resource. This resource has a list of properties. Properties 
 * are named and have a certain type. The type can either be another resource (compare
 * a file that was an attachment from an email) or a literal (this means for example a
 * string, or an integer; the comment mentioned earlier would be a string literal).
 * Each property can either have a cadinality of 1 (again a file can only be saved from
 * one email) or greater than 1 (i.e. infinite, like one file can have arbitraty many
 * associated comments). See \ref hacking on how KMetaData handles literals and cardinalities
 * greater than 1.
 *
 * \section toc Further Reading
 *
 * - \ref page_ontology
 * - \ref hacking
 * - \ref examples
 */


/**
 * \page hacking Using KMetaData
 * 
 * \section hacking_resources Handling Resources
 *
 * In general there are two ways of using KMetaData.
 *
 * -# The preferred way: use the Resource subclasses as generated from \ref page_ontology
 *    This is also the much simpler way since KMetaData takes care of all type casting and
 *    list handling automatically.
 * -# Using Nepomuk::KMetaData::Resource directly. This is much harder since in this case 
 *    the type names (i.e. their URIs as defined in \ref page_ontology) have to be known.
 *    On the other hand it allows to use additional resource types not defined in the ontology
 *    and handle resources without knowing their type.
 *
 * Since all resource classes are derived from Nepomuk::KMetaData::Resource and only add additional methods
 * both ways can be used interchangably.
 *
 * Resource objects (and thus also all objects of classes derived from Resource) with the same 
 * URI share their data. Thus, if one is changed the other one is, too.
 *
 *
 * \subsection hacking_theeasyway Using Resource Subclasses
 *
 * Using Resource subclasses directly is very simple. All that is necessary to handle a
 * resource is to know its type and its URI (the URI can vary a lot between resource types;
 * The simplest example is certainly a local file: the URI is the path to the file).
 *
 * To access or create meta data for a resource one simply creates an instance of the 
 * corresponding class and passes the resource URI to its constructor.
 *
 * In case of a file this would look as follows.
 *
 * \code
 * Nepomuk::KMetaData::File f( "/home/foo/bar.txt" );
 * \endcode
 *
 * Now meta data can be read and set via the methods provided by Nepomuk::KMetaData::File
 * such as Nepomuk::KMetaData::File::setAnnotation.
 *
 * Each resource class also provides a static method which returns all existing instances
 * of this type. This includes instances in the store as well as locally non-synced objects.
 * See Nepomuk::KMetaData::File::allFiles for an example.
 *
 *
 * \subsection hacking_thehardway Using Resource Directly
 *
 * Using the Nepomuk::KMetaData::Resource class directly forces one to learn a little more about 
 * the internals of KMetaData. Resource provides four methods to handle the properties of a
 * resource (reminder: all Resource subclasses as generated from \ref page_ontology are based
 * one these methods):
 *
 * - Nepomuk::KMetaData::Resource::getProperty
 * - Nepomuk::KMetaData::Resource::setProperty
 * - Nepomuk::KMetaData::Resource::removeProperty
 * - Nepomuk::KMetaData::Resource::allProperties
 *
 * Each property's value is represented by a Nepomuk::KMetaData::Variant object which can contain
 * another Resource or a literal (string, int, ...). Other than with the Resource subclasses no
 * automatic type conversion is performed.
 *
 * In case of a property that can have multiple values (cardinality greater than 1) Nepomuk::KMetaData::Resource::setProperty
 * has to be called with a list to set more than one (the Resource subclasses simplify this by
 * adding add methods in addition to the set method) and Nepomuk::KMetaData::Resource::getProperty
 * will also return a list (in both cases encapsulated in a Variant object).
 *
 * When creating a Resource object there are two cases that are dealt with differently:
 *
 * - The resource does not exist yet, i.e. no information about it is stored. In this case KMetaData
 *   does not know the type of the resource and will fall back to %http://www.w3.org/2000/01/rdf-schema\#Resource.
 * - If the resource already exists the type may be empty. It will then be read from the local meta
 *   data store (where it was saved before by KMetaData automatically).
 *
 * As a rule of thumb one should always define the type when creating meta data and leave it empty when
 * reading meta data.
 *
 * When using the plain Nepomuk::KMetaData::Resource class one is completely free to choose the 
 * resource URIs, the type URIs, and the property URIs. However, to preserve compatibility with
 * other applications one is encouraged to stick to those define in \ref page_ontology.
 *
 * \section hacking_general KMetaData Details
 *
 * KMetaData is designed so the user (the developer of a client application) does not have to care about loading or
 * saving the data. Unless auto syncing is disabled via Nepomuk::KMetaData::ResourceManager::setAutoSync meta data
 * is automatically synced with the local Nepomuk meta data store.
 * (Currently the autosync feature is only partially implemented. Data will be
 * synced once the last instance of a resource is deleted.)
 *
 * Although in normal operation it is sufficient to only work with Nepomuk::KMetaData::Resource and
 * its subclasses errors might occur. This is where the Nepomuk::KMetaData::ResourceManager comes
 * in: it provides the Nepomuk::KMetaData::ResourceManager::init method which can be called manually
 * (the resource manager will be initialized automatically anyway) to check if the initialization was
 * successful and KMetaData can be used. In addition it provides the Nepomuk::KMetaData::ResourceManager::error
 * signal which is emitted whenever an error occurs. Errors include failed syncing or loading of meta
 * data.
 *
 *
 * See \ref examples for further examples.
 */


/**
 * \page examples KMetaData Examples
 *
 * Add an annotation (a comment) to a file.
 *
 * \code
 * Nepomuk::KMetaData::File f( "/home/foo/bar.txt" );
 * f.setAnnotation( "This is just a test file that contains nothing of interest." );
 * \endcode
 *
 * The following example is a little more complicated since it contains creating
 * a new tag and thus, coming up with a unique URi for this tag (future versions
 * of KMetaData should provide a way to create a unique URI automatically.)
 *
 * \code
 * Nepomuk::KMetaData::Tag tag( "someURI/that/is.unique" );
 * tag.setName( "Important" );
 * Nepomuk::KMetaData::File f( "/home/foo/bar.txt" );
 * f.addTag( tag );
 * \endcode
 *
 * Reading the information using plain Resource methods:
 *
 * \code
 * Nepomuk::KMetaData::Resource f( "/home/foo/bar.txt" );
 * QString annotation = f.getProperty( "http://nepomuk-kde.semanticdesktop.org/ontology/nkde-0.1#hasAnnotation" ).toString();
 * QList<Resource> tags = f.getProperty( "http://nepomuk-kde.semanticdesktop.org/ontology/nkde-0.1#hasTag" ).toResourceList();
 * QListIterator<Resource> it( tags );
 * while( it.hasNext() )
 *   kdDebug() << "File tagged with tag: "
 *             << it.next().getProperty( "http://nepomuk-kde.semanticdesktop.org/ontology/nkde-0.1#hasName" ).toString();
 * \endcode
 *
 * Reading the information using the convinience classes:
 *
 * \code
 * Nepomuk::KMetaData::File f( "/home/foo/bar.txt" );
 * QString annotation = f.getAnnotation();
 * QList<Tag> tags = f.getTags();
 * QListIterator<Tag> it( tags );
 * while( it.hasNext() )
 *   kdDebug() << "File tagged with tag: " << it.next().getName();
 * \endcode
 *
 * Present all defined properties of an arbitrary resource to the user:
 *
 * \code
 * Nepomuk::KMetaData::Ontology* ont = Nepomuk::KMetaData::ResourceManager::instance()->ontology();
 * Nepomuk::KMetaData::Resource f( "/home/foo/bar.txt" );
 * QHash<QString, Variant> properties = f.allProperties();
 * QHashIterator<QString, Variant> it( properties );
 * while( it.hasNext() ) {
 *   it.next();
 *   kdDebug() << ont->propertyName( it.key() ) << ": " << it.value().toString() << endl;
 * }
 * \endcode
 */

/**
 * \page page_ontology The Nepomuk Desktop Ontology
 *
 * The Nepomuk desktop ontology defines the possible resource types and the relations
 * between them.
 *
 * FIXME: add information about NRL and the actual Nepomuk desktop ontology
 */

#include <kmetadata/resource.h>
#include <kmetadata/variant.h>
#include <kmetadata/resourcemanager.h>
#include <kmetadata/ontology.h>
#include <kmetadata/allresources.h>
