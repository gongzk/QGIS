/***************************************************************************
                              qgscomposition.h
                             -------------------
    begin                : January 2005
    copyright            : (C) 2005 by Radim Blazek
    email                : blazek@itc.it
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSCOMPOSITION_H
#define QGSCOMPOSITION_H

#include <memory>

#include <QDomDocument>
#include <QGraphicsScene>
#include <QLinkedList>
#include <QList>
#include <QPair>
#include <QSet>
#include <QUndoStack>
#include <QPrinter>
#include <QPainter>

#include "qgsaddremoveitemcommand.h"
#include "qgscomposeritemcommand.h"
#include "qgscomposermultiframecommand.h"
#include "qgsatlascomposition.h"
#include "qgspaperitem.h"
#include "qgscomposerobject.h"
#include "qgscomposeritem.h"
#include "qgsobjectcustomproperties.h"

class QgisApp;
class QgsComposerFrame;
class QgsComposerMap;
class QGraphicsRectItem;
class QgsMapRenderer;
class QDomElement;
class QgsComposerArrow;
class QgsComposerPolygon;
class QgsComposerPolyline;
class QgsComposerMouseHandles;
class QgsComposerHtml;
class QgsComposerTableV2;
class QgsComposerItem;
class QgsComposerItemGroup;
class QgsComposerLabel;
class QgsComposerLegend;
class QgsComposerMap;
class QgsComposerPicture;
class QgsComposerScaleBar;
class QgsComposerShape;
class QgsComposerAttributeTable;
class QgsComposerAttributeTableV2;
class QgsComposerMultiFrame;
class QgsComposerMultiFrameCommand;
class QgsVectorLayer;
class QgsComposer;
class QgsFillSymbolV2;
class QgsDataDefined;
class QgsComposerModel;

/** \ingroup core
 * Graphics scene for map printing. The class manages the paper item which always
 * is the item in the back (z-value 0). It maintains the z-Values of the items and stores
 * them in a list in ascending z-Order. This list can be changed to lower/raise items one position
 * or to bring them to front/back.
 * */
class CORE_EXPORT QgsComposition : public QGraphicsScene
{
    Q_OBJECT
  public:

    /** \brief Plot type */
    enum PlotStyle
    {
      Preview = 0, // Use cache etc
      Print,       // Render well
      Postscript   // Fonts need different scaling!
    };

    /** Style to draw the snapping grid*/
    enum GridStyle
    {
      Solid,
      Dots,
      Crosses
    };

    enum ZValueDirection
    {
      ZValueBelow,
      ZValueAbove
    };

    enum PaperOrientation
    {
      Portrait,
      Landscape
    };

    //! @deprecated since 2.4 - use the constructor with QgsMapSettings
    Q_DECL_DEPRECATED QgsComposition( QgsMapRenderer* mapRenderer );
    explicit QgsComposition( const QgsMapSettings& mapSettings );

    /** Composition atlas modes*/
    enum AtlasMode
    {
      AtlasOff,     // Composition is not being controlled by an atlas
      PreviewAtlas, // An atlas composition is being previewed in the app
      ExportAtlas   // The composition is being exported as an atlas
    };

    ~QgsComposition();

    /** Changes size of paper item.
     * @param width page width in mm
     * @param height page height in mm
     * @param keepRelativeItemPosition if true, all items and guides will be moved so that they retain
     * their same relative position to the top left corner of their current page.
     * @see paperHeight
     * @see paperWidth
     */
    void setPaperSize( double width, double height,
                       bool keepRelativeItemPosition = true );

    /** Height of paper item
     * @returns height in mm
     * @see paperWidth
     * @see setPaperSize
     */
    double paperHeight() const;

    /** Width of paper item
     * @returns width in mm
     * @see paperHeight
     * @see setPaperSize
     */
    double paperWidth() const;

    /** Resizes the composition page to fit the current contents of the composition.
     * Calling this method resets the number of pages to 1, with the size set to the
     * minimum size required to fit all existing composer items. Items will also be
     * repositioned so that the new top-left bounds of the composition is at the point
     * (marginLeft, marginTop). An optional margin can be specified.
     * @param marginTop top margin (millimeters)
     * @param marginRight right margin  (millimeters)
     * @param marginBottom bottom margin  (millimeters)
     * @param marginLeft left margin (millimeters)
     * @note added in QGIS 2.12
     * @see setResizeToContentsMargins()
     * @see resizeToContentsMargins()
     */
    void resizePageToContents( double marginTop = 0.0, double marginRight = 0.0,
                               double marginBottom = 0.0, double marginLeft = 0.0 );

    /** Sets the resize to contents margins. These margins are saved in the composition
     * so that they can be restored with the composer.
     * @param marginTop top margin (millimeters)
     * @param marginRight right margin  (millimeters)
     * @param marginBottom bottom margin  (millimeters)
     * @param marginLeft left margin (millimeters)
     * @note added in QGIS 2.12
     * @see resizePageToContents()
     * @see resizeToContentsMargins()
     */
    void setResizeToContentsMargins( double marginTop, double marginRight,
                                     double marginBottom, double marginLeft );

    /** Returns the resize to contents margins. These margins are saved in the composition
     * so that they can be restored with the composer.
     * @param marginTop reference for top margin (millimeters)
     * @param marginRight reference for right margin  (millimeters)
     * @param marginBottom reference for bottom margin  (millimeters)
     * @param marginLeft reference for left margin (millimeters)
     * @note added in QGIS 2.12
     * @see resizePageToContents()
     * @see setResizeToContentsMargins()
     */
    void resizeToContentsMargins( double& marginTop, double& marginRight,
                                  double& marginBottom, double& marginLeft ) const;

    /** Returns the vertical space between pages in a composer view
     * @returns space between pages in mm
     */
    double spaceBetweenPages() const { return mSpaceBetweenPages; }

    /** Sets the number of pages for the composition.
     * @param pages number of pages
     * @see numPages
     */
    void setNumPages( const int pages );

    /** Returns the number of pages in the composition.
     * @returns number of pages
     * @see setNumPages
     */
    int numPages() const;

    /** Returns whether a page is empty, ie, it contains no items except for the background
     * paper item.
     * @param page page number, starting with 1
     * @returns true if page is empty
     * @note added in QGIS 2.5
     * @see numPages
     * @see setNumPages
     * @see shouldExportPage
     */
    bool pageIsEmpty( const int page ) const;

    /** Returns whether a specified page number should be included in exports of the composition.
     * @param page page number, starting with 1
     * @returns true if page should be exported
     * @note added in QGIS 2.5
     * @see numPages
     * @see pageIsEmpty
     */
    bool shouldExportPage( const int page ) const;

    /** Note: added in version 2.1*/
    void setPageStyleSymbol( QgsFillSymbolV2* symbol );
    /** Note: added in version 2.1*/
    QgsFillSymbolV2* pageStyleSymbol() { return mPageStyleSymbol; }

    /** Returns the position within a page of a point in the composition
      @note Added in QGIS 2.1
     */
    QPointF positionOnPage( QPointF position ) const;

    /** Returns the page number corresponding to a point in the composition
      @note Added in QGIS 2.1
     */
    int pageNumberForPoint( QPointF position ) const;

    /** Sets the status bar message for the composer window
      @note Added in QGIS 2.1
     */
    void setStatusMessage( const QString & message );

    /** Refreshes the composition when composer related options change
     @note added in version 2.1
     */
    void updateSettings();

    void setSnapToGridEnabled( const bool b );
    bool snapToGridEnabled() const {return mSnapToGrid;}

    void setGridVisible( const bool b );
    bool gridVisible() const {return mGridVisible;}

    /** Hides / shows custom snap lines*/
    void setSnapLinesVisible( const bool visible );
    bool snapLinesVisible() const {return mGuidesVisible;}

    void setAlignmentSnap( const bool s ) { mAlignmentSnap = s; }
    bool alignmentSnap() const { return mAlignmentSnap; }

    void setSmartGuidesEnabled( const bool b ) { mSmartGuides = b; }
    bool smartGuidesEnabled() const {return mSmartGuides;}

    /** Sets whether the page items should be visible in the composition. Removing
     * them will prevent both display of the page boundaries in composer views and
     * will also prevent them from being rendered in composition exports.
     * @param visible set to true to show pages, false to hide pages
     * @note added in QGIS 2.12
     * @see pagesVisible()
     */
    void setPagesVisible( bool visible );

    /** Returns whether the page items are be visible in the composition. This setting
     * effects both display of the page boundaries in composer views and
     * whether they will be rendered in composition exports.
     * @note added in QGIS 2.12
     * @see setPagesVisible()
     */
    bool pagesVisible() const { return mPagesVisible; }

    /** Removes all snap lines*/
    void clearSnapLines();

    void setSnapGridResolution( const double r );
    double snapGridResolution() const {return mSnapGridResolution;}

    void setSnapGridOffsetX( const double offset );
    double snapGridOffsetX() const {return mSnapGridOffsetX;}

    void setSnapGridOffsetY( const double offset );
    double snapGridOffsetY() const {return mSnapGridOffsetY;}

    void setGridPen( const QPen& p );
    const QPen& gridPen() const {return mGridPen;}

    void setGridStyle( const GridStyle s );
    GridStyle gridStyle() const {return mGridStyle;}

    /** Sets the snap tolerance to use when automatically snapping items during movement and resizing to the
     * composition grid.
     * @param tolerance snap tolerance in pixels
     * @see snapGridTolerance
     * @deprecated Use setSnapTolerance instead
     */
    Q_DECL_DEPRECATED void setSnapGridTolerance( double tolerance ) { mSnapTolerance = tolerance; }

    /** Returns the snap tolerance to use when automatically snapping items during movement and resizing to the
     * composition grid.
     * @returns snap tolerance in pixels
     * @see setSnapGridTolerance
     * @deprecated Use snapTolerance instead
     */
    Q_DECL_DEPRECATED double snapGridTolerance() const {return mSnapTolerance;}

    /** Sets the snap tolerance to use when automatically snapping items during movement and resizing to guides
     * and the edges and centers of other items.
     * @param t snap tolerance in pixels
     * @see alignmentSnapTolerance
     * @deprecated Use setSnapTolerance instead
     */
    Q_DECL_DEPRECATED void setAlignmentSnapTolerance( double t ) { mSnapTolerance = t; }

    /** Returns the snap tolerance to use when automatically snapping items during movement and resizing to guides
     * and the edges and centers of other items.
     * @returns snap tolerance in pixels
     * @see setAlignmentSnapTolerance
     * @deprecated Use snapTolerance instead
     */
    Q_DECL_DEPRECATED double alignmentSnapTolerance() const { return mSnapTolerance; }

    /** Sets the snap tolerance to use when automatically snapping items during movement and resizing to guides
     * and the edges and centers of other items.
     * @param snapTolerance snap tolerance in pixels
     * @see alignmentSnapTolerance
     * @note Added in QGIS 2.5
     */
    void setSnapTolerance( const int snapTolerance ) { mSnapTolerance = snapTolerance; }

    /** Returns the snap tolerance to use when automatically snapping items during movement and resizing to guides
     * and the edges and centers of other items.
     * @returns snap tolerance in pixels
     * @see setAlignmentSnapTolerance
     * @note Added in QGIS 2.5
     */
    int snapTolerance() const { return mSnapTolerance; }

    /** Sets whether selection bounding boxes should be shown in the composition
     * @param boundsVisible set to true to show selection bounding box
     * @see boundingBoxesVisible
     * @note added in QGIS 2.7
     */
    void setBoundingBoxesVisible( const bool boundsVisible );

    /** Returns whether selection bounding boxes should be shown in the composition
     * @returns true if selection bounding boxes should be shown
     * @see setBoundingBoxesVisible
     * @note added in QGIS 2.7
     */
    bool boundingBoxesVisible() const { return mBoundingBoxesVisible; }

    /** Returns pointer to undo/redo command storage*/
    QUndoStack* undoStack() { return mUndoStack; }

    /** Returns the topmost composer item at a specified position. Ignores paper items.
     * @param position point to search for item at
     * @param ignoreLocked set to true to ignore locked items
     * @returns composer item at position
     */
    QgsComposerItem* composerItemAt( QPointF position, const bool ignoreLocked = false ) const;

    /** Returns the topmost composer item at a specified position which is below a specified item. Ignores paper items.
     * @param position point to search for item at
     * @param belowItem item to search below
     * @param ignoreLocked set to true to ignore locked items
     * @returns composer item at position which is below specified item
     */
    QgsComposerItem* composerItemAt( QPointF position, const QgsComposerItem* belowItem, const bool ignoreLocked = false ) const;

    /** Returns the page number (0-based) given a coordinate */
    int pageNumberAt( QPointF position ) const;

    /** Returns on which page number (0-based) is displayed an item */
    int itemPageNumber( const QgsComposerItem* ) const;

    /** Returns list of selected composer items
     * @param includeLockedItems set to true to include locked items in list
     * @returns list of selected items
     */
    QList<QgsComposerItem*> selectedComposerItems( const bool includeLockedItems = true );

    /** Returns pointers to all composer maps in the scene
      @note available in python bindings only with PyQt >= 4.8.4
      */
    QList<const QgsComposerMap*> composerMapItems() const;

    /** Return composer items of a specific type
     * @param itemList list of item type to store matching items in
     * @note not available in python bindings
     */
    template<class T> void composerItems( QList<T*>& itemList );

    /** Return composer items of a specific type on a specified page
     * @param itemList list of item type to store matching items in
     * @param pageNumber page number (0 based)
     * @note not available in python bindings
     * @note added in QGIS 2.5
     */
    template<class T> void composerItemsOnPage( QList<T*>& itemList, const int pageNumber ) const;

    /** Returns the composer map with specified id
     * @return QgsComposerMap or 0 pointer if the composer map item does not exist
     */
    const QgsComposerMap* getComposerMapById( const int id ) const;

    /** Returns the composer html with specified id (a string as named in the
     * composer user interface item properties).
     * @param item the item.
     * @return QgsComposerHtml pointer or 0 pointer if no such item exists.
     * @deprecated Use QgsComposerFrame::multiFrame() instead
     */
    Q_DECL_DEPRECATED const QgsComposerHtml* getComposerHtmlByItem( QgsComposerItem *item ) const;

    /** Returns a composer item given its text identifier.
     *  Ids are not necessarely unique, but this function returns only one element.
     * @param theId - A QString representing the identifier of the item to retrieve.
     * @return QgsComposerItem pointer or 0 pointer if no such item exists.
     */
    const QgsComposerItem* getComposerItemById( const QString& theId ) const;

    /** Returns a composer item given its unique identifier.
     * @param theUuid A QString representing the UUID of the item to
     */
    const QgsComposerItem* getComposerItemByUuid( const QString& theUuid ) const;

    int printResolution() const {return mPrintResolution;}
    void setPrintResolution( const int dpi );

    bool printAsRaster() const {return mPrintAsRaster;}
    void setPrintAsRaster( const bool enabled ) { mPrintAsRaster = enabled; }

    /** Returns true if the composition will generate corresponding world files when pages
     * are exported.
     * @see setGenerateWorldFile()
     * @see worldFileMap()
     */
    bool generateWorldFile() const { return mGenerateWorldFile; }

    /** Sets whether the composition will generate corresponding world files when pages
     * are exported.
     * @param enabled set to true to generate world files
     * @see generateWorldFile()
     * @see setWorldFileMap()
     */
    void setGenerateWorldFile( bool enabled ) { mGenerateWorldFile = enabled; }

    /** Returns the map item which will be used to generate corresponding world files when the
     * composition is exported, or nullptr if no corresponding map is set.
     * @see setWorldFileMap()
     * @see generateWorldFile()
     */
    QgsComposerMap* worldFileMap() const;

    /** Sets the map item which will be used to generate corresponding world files when the
     * composition is exported.
     * @param map composer map item
     * @see worldFileMap()
     * @see setGenerateWorldFile()
     */
    void setWorldFileMap( QgsComposerMap* map );

    /** Returns true if a composition should use advanced effects such as blend modes */
    bool useAdvancedEffects() const {return mUseAdvancedEffects;}
    /** Used to enable or disable advanced effects such as blend modes in a composition */
    void setUseAdvancedEffects( const bool effectsEnabled );

    /** Returns pointer to map renderer of qgis map canvas*/
    //! @deprecated since 2.4 - use mapSettings() instead. May return null if not initialized with QgsMapRenderer
    Q_DECL_DEPRECATED QgsMapRenderer* mapRenderer() { return mMapRenderer; }

    //! Return setting of QGIS map canvas
    //! @note added in 2.4
    const QgsMapSettings& mapSettings() const { return mMapSettings; }

    QgsComposition::PlotStyle plotStyle() const { return mPlotStyle; }
    void setPlotStyle( const QgsComposition::PlotStyle style ) { mPlotStyle = style; }

    /** Returns the mm font size for a font that has point size set.
     * Each item that sets a font should call this function before drawing text
     * @deprecated use QgsComposerUtils::pointsToMM instead
     */
    Q_DECL_DEPRECATED int pixelFontSize( double pointSize ) const;

    /** Does the inverse calculation and returns points for mm
     * @deprecated use QgsComposerUtils::mmToPoints instead
     */
    Q_DECL_DEPRECATED double pointFontSize( int pixelSize ) const;

    /** Writes settings to xml (paper dimension)*/
    bool writeXML( QDomElement& composerElem, QDomDocument& doc );

    /** Reads settings from xml file*/
    bool readXML( const QDomElement& compositionElem, const QDomDocument& doc );

    /** Load a template document
     * @param doc template document
     * @param substitutionMap map with text to replace. Text needs to be enclosed by brackets (e.g. '[text]' )
     * @param addUndoCommands whether or not to add undo commands
     * @param clearComposition set to true to clear the existing composition and read all composition and
     * atlas properties from the template. Set to false to only add new items from the template, without
     * overwriting the existing items or composition settings.
     */
    bool loadFromTemplate( const QDomDocument& doc, QMap<QString, QString>* substitutionMap = nullptr,
                           bool addUndoCommands = false, const bool clearComposition = true );

    /** Add items from XML representation to the graphics scene (for project file reading, pasting items from clipboard)
     * @param elem items parent element, e.g. \verbatim <Composer> \endverbatim or \verbatim <ComposerItemClipboard> \endverbatim
     * @param doc xml document
     * @param mapsToRestore for reading from project file: set preview move 'rectangle' to all maps and save the preview states to show composer maps on demand
     * @param addUndoCommands insert AddItem commands if true (e.g. for copy/paste)
     * @param pos item position. Optional, take position from xml if 0
     * @param pasteInPlace whether the position should be kept but mapped to the page origin. (the page is the page under to the mouse cursor)
     * @note parameters mapsToRestore, addUndoCommands pos and pasteInPlace not available in python bindings
     */
    void addItemsFromXML( const QDomElement& elem, const QDomDocument& doc, QMap< QgsComposerMap*, int >* mapsToRestore = nullptr,
                          bool addUndoCommands = false, QPointF* pos = nullptr, bool pasteInPlace = false );

    /** Adds item to z list. Usually called from constructor of QgsComposerItem*/
    void addItemToZList( QgsComposerItem* item );
    /** Removes item from z list. Usually called from destructor of QgsComposerItem*/
    void removeItemFromZList( QgsComposerItem* item );

    //functions to move selected items in hierarchy
    void raiseSelectedItems();

    //returns true if successful
    bool raiseItem( QgsComposerItem* item );
    void lowerSelectedItems();
    //returns true if successful
    bool lowerItem( QgsComposerItem* item );
    void moveSelectedItemsToTop();
    //returns true if successful
    bool moveItemToTop( QgsComposerItem* item );
    void moveSelectedItemsToBottom();
    //returns true if successful
    bool moveItemToBottom( QgsComposerItem* item );

    //functions to find items by their position in the z list
    void selectNextByZOrder( const ZValueDirection direction );
    QgsComposerItem* getComposerItemBelow( QgsComposerItem* item ) const;
    QgsComposerItem* getComposerItemAbove( QgsComposerItem* item ) const;

    //functions to align selected items
    void alignSelectedItemsLeft();
    void alignSelectedItemsHCenter();
    void alignSelectedItemsRight();
    void alignSelectedItemsTop();
    void alignSelectedItemsVCenter();
    void alignSelectedItemsBottom();

    //functions to lock and unlock items
    /** Lock the selected items*/
    void lockSelectedItems();
    /** Unlock all items*/
    void unlockAllItems();

    /** Creates a new group from a list of composer items and adds it to the composition.
     * @param items items to include in group
     * @returns QgsComposerItemGroup of grouped items, if grouping was possible
     * @note added in QGIS 2.6
     */
    QgsComposerItemGroup* groupItems( QList<QgsComposerItem*> items );

    /** Ungroups items by removing them from an item group and removing the group from the
     * composition.
     * @param group item group to ungroup
     * @returns list of items removed from the group, or an empty list if ungrouping
     * was not successful
     * @note added in QGIS 2.6
     */
    QList<QgsComposerItem*> ungroupItems( QgsComposerItemGroup* group );

    /** Sorts the zList. The only time where this function needs to be called is from QgsComposer
     * after reading all the items from xml file
     * @deprecated use refreshZList instead
     */
    Q_DECL_DEPRECATED void sortZList() {}

    /** Rebuilds the z order list by adding any item which are present in the composition
     * but missing from the z order list.
     */
    void refreshZList();

    /** Snaps a scene coordinate point to grid*/
    QPointF snapPointToGrid( QPointF scenePoint ) const;

    /** Returns pointer to snap lines collection*/
    QList< QGraphicsLineItem* >* snapLines() {return &mSnapLines;}

    /** Returns pointer to selection handles
     * @note not available in python bindings
     */
    QgsComposerMouseHandles* selectionHandles() {return mSelectionHandles;}

    /** Add a custom snap line (can be horizontal or vertical)*/
    QGraphicsLineItem* addSnapLine();
    /** Remove custom snap line (and delete the object)*/
    void removeSnapLine( QGraphicsLineItem* line );
    /** Get nearest snap line
     * @note not available in python bindings
     */
    QGraphicsLineItem* nearestSnapLine( const bool horizontal, const double x, const double y, const double tolerance, QList< QPair< QgsComposerItem*, QgsComposerItem::ItemPositionMode > >& snappedItems ) const;

    /** Allocates new item command and saves initial state in it
     * @param item target item
     * @param commandText descriptive command text
     * @param c context for merge commands (unknown for non-mergeable commands)
     */
    void beginCommand( QgsComposerItem* item, const QString& commandText, const QgsComposerMergeCommand::Context c = QgsComposerMergeCommand::Unknown );

    /** Saves end state of item and pushes command to the undo history*/
    void endCommand();
    /** Deletes current command*/
    void cancelCommand();

    void beginMultiFrameCommand( QgsComposerMultiFrame* multiFrame, const QString& text, const QgsComposerMultiFrameMergeCommand::Context c = QgsComposerMultiFrameMergeCommand::Unknown );
    void endMultiFrameCommand();
    /** Deletes current multi frame command*/
    void cancelMultiFrameCommand();

    /** Adds multiframe. The object is owned by QgsComposition until removeMultiFrame is called*/
    void addMultiFrame( QgsComposerMultiFrame* multiFrame );
    /** Removes multi frame (but does not delete it)*/
    void removeMultiFrame( QgsComposerMultiFrame* multiFrame );
    /** Adds an arrow item to the graphics scene and advises composer to create a widget for it (through signal)
      @note not available in python bindings*/
    void addComposerArrow( QgsComposerArrow* arrow );
    /** Adds label to the graphics scene and advises composer to create a widget for it (through signal)*/
    void addComposerLabel( QgsComposerLabel* label );
    /** Adds map to the graphics scene and advises composer to create a widget for it (through signal)*/
    void addComposerMap( QgsComposerMap* map, const bool setDefaultPreviewStyle = true );
    /** Adds scale bar to the graphics scene and advises composer to create a widget for it (through signal)*/
    void addComposerScaleBar( QgsComposerScaleBar* scaleBar );
    /** Adds legend to the graphics scene and advises composer to create a widget for it (through signal)*/
    void addComposerLegend( QgsComposerLegend* legend );
    /** Adds picture to the graphics scene and advises composer to create a widget for it (through signal)*/
    void addComposerPicture( QgsComposerPicture* picture );
    /** Adds a composer shape to the graphics scene and advises composer to create a widget for it (through signal)*/
    void addComposerShape( QgsComposerShape* shape );
    /** Adds a composer polygon and advises composer to create a widget for it (through signal)*/
    void addComposerPolygon( QgsComposerPolygon* polygon );
    /** Adds a composer polyline and advises composer to create a widget for it (through signal)*/
    void addComposerPolyline( QgsComposerPolyline* polyline );
    /** Adds a composer table to the graphics scene and advises composer to create a widget for it (through signal)*/
    void addComposerTable( QgsComposerAttributeTable* table );
    /** Adds composer html frame and advises composer to create a widget for it (through signal)*/
    void addComposerHtmlFrame( QgsComposerHtml* html, QgsComposerFrame* frame );
    /** Adds composer tablev2 frame and advises composer to create a widget for it (through signal)*/
    void addComposerTableFrame( QgsComposerAttributeTableV2* table, QgsComposerFrame* frame );

    /** Remove item from the graphics scene. Additionally to QGraphicsScene::removeItem, this function considers undo/redo command*/
    void removeComposerItem( QgsComposerItem* item, const bool createCommand = true, const bool removeGroupItems = true );

    /** Convenience function to create a QgsAddRemoveItemCommand, connect its signals and push it to the undo stack*/
    void pushAddRemoveCommand( QgsComposerItem* item, const QString& text, const QgsAddRemoveItemCommand::State state = QgsAddRemoveItemCommand::Added );

    /** If true, prevents any mouse cursor changes by the composition or by any composer items
     * Used by QgsComposer and QgsComposerView to prevent unwanted cursor changes
     */
    void setPreventCursorChange( const bool preventChange ) { mPreventCursorChange = preventChange; }
    bool preventCursorChange() const { return mPreventCursorChange; }

    //printing

    /** Prepare the printer for printing */
    void beginPrint( QPrinter& printer, const bool evaluateDDPageSize = false );
    /** Prepare the printer for printing in a PDF */
    void beginPrintAsPDF( QPrinter& printer, const QString& file );

    /** Print on a preconfigured printer
     * @param printer QPrinter destination
     * @param painter QPainter source
     * @param startNewPage set to true to begin the print on a new page
     */
    void doPrint( QPrinter& printer, QPainter& painter, bool startNewPage = false );

    /** Convenience function that prepares the printer and prints
     * @returns true if print was successful
     */
    bool print( QPrinter &printer, const bool evaluateDDPageSize = false );

    /** Convenience function that prepares the printer for printing in PDF and prints
     * @returns true if export was successful
     */
    bool exportAsPDF( const QString& file );

    /** Renders a composer page to an image.
     * @param page page number, 0 based such that the first page is page 0
     * @param imageSize optional target image size, in pixels. It is the caller's responsibility
     * to ensure that the ratio of the target image size matches the ratio of the composition
     * page size.
     * @param dpi optional dpi override, or 0 to use default composition print resolution. This
     * parameter has no effect if imageSize is specified.
     * @returns rendered image, or null image if image does not fit into available memory
     * @see renderRectAsRaster()
     * @see renderPage()
     */
    QImage printPageAsRaster( int page, QSize imageSize = QSize(), int dpi = 0 );

    /** Renders a portion of the composition to an image. This method can be used to render
     * sections of pages rather than full pages.
     * @param rect region of composition to render
     * @param imageSize optional target image size, in pixels. It is the caller's responsibility
     * to ensure that the ratio of the target image size matches the ratio of the specified
     * region of the composition.
     * @param dpi optional dpi override, or 0 to use default composition print resolution. This
     * parameter has no effect if imageSize is specified.
     * @returns rendered image, or null image if image does not fit into available memory
     * @note added in QGIS 2.12
     * @see printPageAsRaster()
     * @see renderRect()
     */
    QImage renderRectAsRaster( const QRectF& rect, QSize imageSize = QSize(), int dpi = 0 );

    /** Renders a full page to a paint device.
     * @param p destination painter
     * @param page page number, 0 based such that the first page is page 0
     * @see renderRect()
     * @see printPageAsRaster()
     */
    void renderPage( QPainter* p, int page );

    /** Renders a portion of the composition to a paint device. This method can be used
     * to render sections of pages rather than full pages.
     * @param p destination painter
     * @param rect region of composition to render
     * @note added in QGIS 2.12
     * @see renderPage()
     * @see renderRectAsRaster()
     */
    void renderRect( QPainter* p, const QRectF& rect );

    /** Georeferences a file (image of PDF) exported from the composition.
     * @param file filename of exported file
     * @param referenceMap map item to use for georeferencing, or leave as nullptr to use the
     * currently defined worldFileMap().
     * @param exportRegion set to a valid rectangle to indicate that only part of the composition was
     * exported
     * @param dpi set to DPI of exported file, or leave as -1 to use composition's DPI.
     * @note added in QGIS 2.16
     */
    void georeferenceOutput( const QString& file, QgsComposerMap* referenceMap = nullptr,
                             const QRectF& exportRegion = QRectF(), double dpi = -1 ) const;

    /** Compute world file parameters. Assumes the whole page containing the associated map item
     * will be exported.
     */
    void computeWorldFileParameters( double& a, double& b, double& c, double& d, double& e, double& f ) const;

    /** Computes the world file parameters for a specified region of the composition.
     * @param exportRegion region of the composition which will be associated with world file
     * @param a
     * @param b
     * @param c
     * @param d
     * @param e
     * @param f
     * @note added in QGIS 2.12
     */
    void computeWorldFileParameters( const QRectF& exportRegion, double& a, double& b, double& c, double& d, double& e, double& f ) const;

    QgsAtlasComposition& atlasComposition() { return mAtlasComposition; }

    /** Resizes a QRectF relative to the change from boundsBefore to boundsAfter
     * @deprecated use QgsComposerUtils::relativeResizeRect instead
     */
    Q_DECL_DEPRECATED static void relativeResizeRect( QRectF& rectToResize, const QRectF& boundsBefore, const QRectF& boundsAfter );

    /** Returns a scaled position given a before and after range
     * @deprecated use QgsComposerUtils::relativePosition instead
     */
    Q_DECL_DEPRECATED static double relativePosition( double position, double beforeMin, double beforeMax, double afterMin, double afterMax );

    /** Returns the current atlas mode of the composition
     * @returns current atlas mode
     * @see setAtlasMode
     */
    QgsComposition::AtlasMode atlasMode() const { return mAtlasMode; }

    /** Sets the current atlas mode of the composition.
     * @param mode atlas mode to switch to
     * @returns false if the mode could not be changed.
     * @see atlasMode
     */
    bool setAtlasMode( const QgsComposition::AtlasMode mode );

    /** Return pages in the correct order
     * @note composerItems(QList< QgsPaperItem* > &) may not return pages in the correct order
     * @note added in version 2.4
     */
    QList< QgsPaperItem* > pages() { return mPages; }

    /** Returns a reference to the data defined settings for one of the composition's data defined properties.
     * @param property data defined property to return
     * @note this method was added in version 2.5
     */
    QgsDataDefined* dataDefinedProperty( const QgsComposerObject::DataDefinedProperty property );

    /** Sets parameters for a data defined property for the composition
     * @param property data defined property to set
     * @param active true if data defined property is active, false if it is disabled
     * @param useExpression true if the expression should be used
     * @param expression expression for data defined property
     * @param field field name if the data defined property should take its value from a field
     * @note this method was added in version 2.5
     */
    void setDataDefinedProperty( const QgsComposerObject::DataDefinedProperty property, bool active, bool useExpression, const QString &expression, const QString &field );

    /** Returns the items model attached to the composition
     * @returns QgsComposerModel for composition
     * @note this method was added in version 2.5
     */
    QgsComposerModel * itemsModel() { return mItemsModel; }

    /** Set a custom property for the composition.
     * @param key property key. If a property with the same key already exists it will be overwritten.
     * @param value property value
     * @see customProperty()
     * @see removeCustomProperty()
     * @see customProperties()
     * @note added in QGIS 2.12
     */
    void setCustomProperty( const QString &key, const QVariant &value );

    /** Read a custom property from the composition.
     * @param key property key
     * @param defaultValue default value to return if property with matching key does not exist
     * @returns value of matching property
     * @see setCustomProperty()
     * @see removeCustomProperty()
     * @see customProperties()
     * @note added in QGIS 2.12
     */
    QVariant customProperty( const QString &key, const QVariant &defaultValue = QVariant() ) const;

    /** Remove a custom property from the composition.
     * @param key property key
     * @see setCustomProperty()
     * @see customProperty()
     * @see customProperties()
     * @note added in QGIS 2.12
     */
    void removeCustomProperty( const QString &key );

    /** Return list of keys stored in custom properties for composition.
     * @see setCustomProperty()
     * @see customProperty()
     * @see removeCustomProperty()
     * @note added in QGIS 2.12
     */
    QStringList customProperties() const;

    /** Returns the bounding box of the items contained on a specified page.
     * @param pageNumber page number, where 0 is the first page
     * @param visibleOnly set to true to only include visible items
     * @note added in QGIS 2.12
     */
    QRectF pageItemBounds( int pageNumber, bool visibleOnly = false ) const;

    /** Calculates the bounds of all non-gui items in the composition. Ignores snap lines and mouse handles.
     * @param ignorePages set to true to ignore page items
     * @param margin optional marginal (in percent, eg 0.05 = 5% ) to add around items
     */
    QRectF compositionBounds( bool ignorePages = false, double margin = 0.0 ) const;

  public slots:
    /** Casts object to the proper subclass type and calls corresponding itemAdded signal*/
    void sendItemAddedSignal( QgsComposerItem* item );

    /** Updates the scene bounds of the composition
    @note added in version 2.2*/
    void updateBounds();

    /** Forces items in the composition to refresh. For instance, this causes maps to redraw
     * and rebuild cached images, html items to reload their source url, and attribute tables
     * to refresh their contents. Calling this also triggers a recalculation of all data defined
     * attributes within the composition.
     * @note added in version 2.3*/
    void refreshItems();

    /** Clears any selected items and sets an item as the current selection.
     * @param item item to set as selected
     * @note added in version 2.3*/
    void setSelectedItem( QgsComposerItem* item );

    /** Clears any selected items in the composition. Call this method rather than
     * QGraphicsScene::clearSelection, as the latter does not correctly emit signals to allow
     * the composition's model to update.
     * @note added in version 2.5*/
    void setAllUnselected();

    /** Refreshes a data defined property for the composition by reevaluating the property's value
     * and redrawing the composition with this new value.
     * @param property data defined property to refresh. If property is set to
     * QgsComposerItem::AllProperties then all data defined properties for the composition will be
     * refreshed.
     * @param context expression context for evaluating data defined expressions
     * @note this method was added in version 2.5
     */
    void refreshDataDefinedProperty( const QgsComposerObject::DataDefinedProperty property = QgsComposerObject::AllProperties, const QgsExpressionContext* context = nullptr );

    /** Creates an expression context relating to the compositions's current state. The context includes
     * scopes for global, project, composition and atlas properties.
     * @note added in QGIS 2.12
     */
    QgsExpressionContext* createExpressionContext() const;

  protected:
    void init();


  private:
    /** Pointer to map renderer of QGIS main map*/
    QgsMapRenderer* mMapRenderer;
    const QgsMapSettings& mMapSettings;

    QgsComposition::PlotStyle mPlotStyle;
    double mPageWidth;
    double mPageHeight;
    QList< QgsPaperItem* > mPages;
    double mSpaceBetweenPages; //space in preview between pages

    /** Drawing style for page*/
    QgsFillSymbolV2* mPageStyleSymbol;
    void createDefaultPageStyleSymbol();

    /** List multiframe objects*/
    QSet<QgsComposerMultiFrame*> mMultiFrames;

    /** Dpi for printout*/
    int mPrintResolution;

    /** Flag if map should be printed as a raster (via QImage). False by default*/
    bool mPrintAsRaster;

    /** Flag if a world file should be generated on raster export */
    bool mGenerateWorldFile;

    /** Item ID for composer map to use for the world file generation */
    QString mWorldFileMapId;

    /** Flag if advanced visual effects such as blend modes should be used. True by default*/
    bool mUseAdvancedEffects;

    /** Parameters for snap to grid function*/
    bool mSnapToGrid;
    bool mGridVisible;
    double mSnapGridResolution;
    double mSnapGridOffsetX;
    double mSnapGridOffsetY;
    QPen mGridPen;
    GridStyle mGridStyle;

    /** Parameters for alignment snap*/
    bool mAlignmentSnap;
    bool mGuidesVisible;
    bool mSmartGuides;
    int mSnapTolerance;

    /** Arbitraty snap lines (horizontal and vertical)*/
    QList< QGraphicsLineItem* > mSnapLines;

    double mResizeToContentsMarginTop;
    double mResizeToContentsMarginRight;
    double mResizeToContentsMarginBottom;
    double mResizeToContentsMarginLeft;

    bool mBoundingBoxesVisible;
    bool mPagesVisible;
    QgsComposerMouseHandles* mSelectionHandles;

    QUndoStack* mUndoStack;

    QgsComposerItemCommand* mActiveItemCommand;
    QgsComposerMultiFrameCommand* mActiveMultiFrameCommand;

    /** The atlas composition object. It is held by the QgsComposition */
    QgsAtlasComposition mAtlasComposition;

    QgsComposition::AtlasMode mAtlasMode;

    bool mPreventCursorChange;

    QgsComposerModel * mItemsModel;

    /** Map of data defined properties for the composition to string name to use when exporting composition to xml*/
    QMap< QgsComposerObject::DataDefinedProperty, QString > mDataDefinedNames;
    /** Map of current data defined properties to QgsDataDefined for the composition*/
    QMap< QgsComposerObject::DataDefinedProperty, QgsDataDefined* > mDataDefinedProperties;

    QgsObjectCustomProperties mCustomProperties;

    QgsComposition(); //default constructor is forbidden

    /** Reset z-values of items based on position in z list*/
    void updateZValues( const bool addUndoCommands = true );

    /** Returns the bounding rectangle of the selected items in scene coordinates
     @return 0 in case of success*/
    int boundingRectOfSelectedItems( QRectF& bRect );

    /** Loads default composer settings*/
    void loadDefaults();

    /** Loads composer settings which may change, eg grid color*/
    void loadSettings();

    /** Calculates the item minimum position from an xml string*/
    QPointF minPointFromXml( const QDomElement& elem ) const;

    void connectAddRemoveCommandSignals( QgsAddRemoveItemCommand* c );

    void updatePaperItems();
    void addPaperItem();
    void removePaperItems();
    void deleteAndRemoveMultiFrames();

    static QString encodeStringForXML( const QString& str );

    //tries to return the current QGraphicsView attached to the composition
    QGraphicsView* graphicsView() const;

    /** Recalculates the page size using data defined page settings
     * @param context expression context for data defined page sizes
     */
    void refreshPageSize( const QgsExpressionContext* context = nullptr );

    /** Evaluate a data defined property and return the calculated value
     * @returns true if data defined property could be successfully evaluated
     * @param property data defined property to evaluate
     * @param expressionValue QVariant for storing the evaluated value
     * @param context expression context for evaluating expressions. Must have feature and fields set
     * to corresponding atlas feature and coverage layer fields prior to calling this method.
     * @param dataDefinedProperties map of data defined properties to QgsDataDefined
     * @note this method was added in version 2.5
     */
    bool dataDefinedEvaluate( QgsComposerObject::DataDefinedProperty property, QVariant &expressionValue,
                              const QgsExpressionContext& context,
                              QMap< QgsComposerObject::DataDefinedProperty, QgsDataDefined* >* dataDefinedProperties );

    /** Returns whether a data defined property has been set and is currently active.
     * @param property data defined property to test
     * @param dataDefinedProperties map of data defined properties to QgsDataDefined
     * @note this method was added in version 2.5
     */
    bool dataDefinedActive( const QgsComposerObject::DataDefinedProperty property,
                            const QMap<QgsComposerObject::DataDefinedProperty, QgsDataDefined *> *dataDefinedProperties ) const;

    /** Evaluates a data defined property and returns the calculated value.
     * @param property data defined property to evaluate
     * @param feature current atlas feature to evaluate property for
     * @param fields fields from atlas layer
     * @param context expression context for evaluating expressions (note, must have fields and feature set)
     * @param dataDefinedProperties map of data defined properties to QgsDataDefined
     * @note this method was added in version 2.5
     */
    QVariant dataDefinedValue( QgsComposerObject::DataDefinedProperty property, const QgsFeature *feature, const QgsFields& fields,
                               const QgsExpressionContext& context,
                               QMap<QgsComposerObject::DataDefinedProperty, QgsDataDefined *> *dataDefinedProperties ) const;


    /** Prepares the expression for a data defined property.
     * @param dd data defined to prepare. If no data defined is set, all data defined expressions will be prepared
     * @param dataDefinedProperties map of data defined properties to QgsDataDefined
     * @param context expression context for expression preparation.  Should have fields set to match the current atlas layer if required prior
     * to calling this method.
     * @note this method was added in version 2.5
     */
    void prepareDataDefinedExpression( QgsDataDefined *dd, QMap< QgsComposerObject::DataDefinedProperty, QgsDataDefined* >* dataDefinedProperties, const QgsExpressionContext& context ) const;

    /** Check whether any data defined page settings are active.
     * @returns true if any data defined page settings are active.
     * @note this method was added in version 2.5
     */
    bool ddPageSizeActive() const;

    /** Computes a GDAL style geotransform for georeferencing a composition.
     * @param referenceMap map item to use for georeferencing, or leave as nullptr to use the
     * currently defined worldFileMap().
     * @param exportRegion set to a valid rectangle to indicate that only part of the composition is
     * being exported
     * @param dpi allows overriding the default composition DPI, or leave as -1 to use composition's DPI.
     * @note added in QGIS 2.16
     */
    double* computeGeoTransform( const QgsComposerMap* referenceMap = nullptr, const QRectF& exportRegion = QRectF(), double dpi = -1 ) const;


  private slots:
    /*Prepares all data defined expressions*/
    void prepareAllDataDefinedExpressions();

  signals:
    void paperSizeChanged();
    void nPagesChanged();

    /** Is emitted when the compositions print resolution changes*/
    void printResolutionChanged();

    /** Is emitted when selected item changed. If 0, no item is selected*/
    void selectedItemChanged( QgsComposerItem* selected );
    /** Is emitted when new composer arrow has been added to the view*/
    void composerArrowAdded( QgsComposerArrow* arrow );
    /** Is emitted when new composer polygon has been added to the view*/
    void composerPolygonAdded( QgsComposerPolygon* polygon );
    /** Is emitted when new composer polyline has been added to the view*/
    void composerPolylineAdded( QgsComposerPolyline* polyline );
    /** Is emitted when a new composer html has been added to the view*/
    void composerHtmlFrameAdded( QgsComposerHtml* html, QgsComposerFrame* frame );
    /** Is emitted when a new item group has been added to the view*/
    void composerItemGroupAdded( QgsComposerItemGroup* group );
    /** Is emitted when new composer label has been added to the view*/
    void composerLabelAdded( QgsComposerLabel* label );
    /** Is emitted when new composer map has been added to the view*/
    void composerMapAdded( QgsComposerMap* map );
    /** Is emitted when new composer scale bar has been added*/
    void composerScaleBarAdded( QgsComposerScaleBar* scalebar );
    /** Is emitted when a new composer legend has been added*/
    void composerLegendAdded( QgsComposerLegend* legend );
    /** Is emitted when a new composer picture has been added*/
    void composerPictureAdded( QgsComposerPicture* picture );
    /** Is emitted when a new composer shape has been added*/
    void composerShapeAdded( QgsComposerShape* shape );
    /** Is emitted when a new composer table has been added*/
    void composerTableAdded( QgsComposerAttributeTable* table );
    /** Is emitted when a new composer table frame has been added to the view*/
    void composerTableFrameAdded( QgsComposerAttributeTableV2* table, QgsComposerFrame* frame );
    /** Is emitted when a composer item has been removed from the scene*/
    void itemRemoved( QgsComposerItem* );

    /** Is emitted when item in the composition must be refreshed*/
    void refreshItemsTriggered();

    /** Is emitted when the composition has an updated status bar message for the composer window*/
    void statusMsgChanged( const QString& message );

    friend class QgsComposerObject; //for accessing dataDefinedEvaluate, readDataDefinedPropertyMap and writeDataDefinedPropertyMap
    friend class QgsComposerModel; //for accessing updateZValues (should not be public)
    friend class TestQgsComposition;
};

template<class T> void QgsComposition::composerItems( QList<T*>& itemList )
{
  itemList.clear();
  QList<QGraphicsItem *> graphicsItemList = items();
  QList<QGraphicsItem *>::iterator itemIt = graphicsItemList.begin();
  for ( ; itemIt != graphicsItemList.end(); ++itemIt )
  {
    T* item = dynamic_cast<T*>( *itemIt );
    if ( item )
    {
      itemList.push_back( item );
    }
  }
}

template<class T> void QgsComposition::composerItemsOnPage( QList<T*>& itemList, const int pageNumber ) const
{
  itemList.clear();
  QList<QGraphicsItem *> graphicsItemList = items();
  QList<QGraphicsItem *>::iterator itemIt = graphicsItemList.begin();
  for ( ; itemIt != graphicsItemList.end(); ++itemIt )
  {
    T* item = dynamic_cast<T*>( *itemIt );
    if ( item && itemPageNumber( item ) == pageNumber )
    {
      itemList.push_back( item );
    }
  }
}


#endif



