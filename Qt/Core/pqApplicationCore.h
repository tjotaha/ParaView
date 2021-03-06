/*=========================================================================

   Program: ParaView
   Module:    $RCSfile$

   Copyright (c) 2005-2008 Sandia Corporation, Kitware Inc.
   All rights reserved.

   ParaView is a free software; you can redistribute it and/or modify it
   under the terms of the ParaView license version 1.2. 

   See License_v1.2.txt for the full ParaView license.
   A copy of this license can be obtained by contacting
   Kitware Inc.
   28 Corporate Drive
   Clifton Park, NY 12065
   USA

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

========================================================================*/
#ifndef __pqApplicationCore_h
#define __pqApplicationCore_h

#include "pqCoreExport.h"
#include "vtkSetGet.h" // for VTK_LEGACY macro
#include <QObject>
#include <QPointer>

class pq3DWidgetFactory;
class pqDisplayPolicy;
class pqLinksModel;
class pqLookupTableManager;
class pqObjectBuilder;
class pqOptions;
class pqOutputWindow;
class pqOutputWindowAdapter;
class pqPipelineSource;
class pqPluginManager;
class pqProgressManager;
class pqServer;
class pqServerManagerModel;
class pqServerManagerObserver;
class pqServerManagerSelectionModel;
class pqServerResource;
class pqServerResources;
class pqServerStartups;
class pqSettings;
class pqTestUtility;
class pqUndoStack;
class QApplication;
class QStringList;
class vtkPVXMLElement;
class vtkSMGlobalPropertiesManager;
class vtkSMProxyLocator;

/// This class is the crux of the ParaView application. It creates
/// and manages various managers which are necessary for the ParaView-based
/// client to work with the ServerManager.
/// For clients based of the pqCore library,
/// simply instantiate this pqApplicationCore after QApplication initialization 
/// and then create your main window etc. like a standard Qt application. You can then
/// use the facilities provided by pqCore such as the pqObjectBuilder,
/// pqUndoStack etc. in your application. After that point.
class PQCORE_EXPORT pqApplicationCore : public QObject
{
  Q_OBJECT
public:
  // Get the global instace for the pqApplicationCore.
  static pqApplicationCore* instance();

  /// DEPRECATED CONSTRUCTOR. This expects the old-style
  /// pqProcessModuleGUIHelper shebang. Only here till all application switch to
  /// new style.
  /// @deprecated Use pqApplicationCore(int, char**, pqOptions*, QObject*)
  /// instead which uses the new style application initialization process.
  VTK_LEGACY(pqApplicationCore(QObject* parentObject=NULL));

  /// Preferred constructor. Initializes the server-manager engine and sets up
  /// the core functionality. If application supports special command line
  /// options, pass an instance of pqOptions subclass to the constructor,
  /// otherwise a new instance of pqOptions with standard ParaView command line
  /// options will be created.
  pqApplicationCore(int& argc, char** argv, pqOptions* options=0, QObject* parent=0);

  /// Dangerous option that disables the debug output window, intended for
  /// demo purposes only
  void disableOutputWindow();

  /// Provides access to the command line options object.
  pqOptions* getOptions() const
    { return this->Options; }

  /// Get the Object Builder. Object Buider must be used
  /// to create complex objects such as sources, filters,
  /// readers, views, displays etc.
  pqObjectBuilder* getObjectBuilder() const
    { return this->ObjectBuilder; }

  /// Set/Get the application's central undo stack. By default no undo stack is
  /// provided. Applications must set on up as required.
  void setUndoStack(pqUndoStack* stack);
  pqUndoStack* getUndoStack() const
    { return this->UndoStack; }

  /// Custom Applications may need use various "managers"
  /// All such manager can be registered with the pqApplicationCore
  /// so that that can be used by other components of the 
  /// application. Registering with pqApplicationCore gives easy
  /// access to these managers from the application code. Note 
  /// that custom applications are not required to register managers.
  /// However certain optional components of the pqCore may
  /// expect some managers.
  /// Only one manager can be registered for a \c function.
  void registerManager(const QString& function, QObject* manager);

  /// Returns a manager for a particular function, if any.
  //. \sa registerManager
  QObject* manager(const QString& function);

  /// Unregisters a manager for a particular function, if any.
  void unRegisterManager(const QString& function);

  /// ServerManagerObserver observer the vtkSMProxyManager
  /// for changes to the server manager and fires signals on
  /// certain actions such as registeration/unregistration of proxies
  /// etc. Returns the ServerManagerObserver used by the application.
  pqServerManagerObserver* getServerManagerObserver()
    { return this->ServerManagerObserver; }

  /// ServerManagerModel is the representation of the ServerManager
  /// using pqServerManagerModelItem subclasses. It makes it possible to
  /// explore the ServerManager with ease by separating proxies based 
  /// on their functionality/type.
  pqServerManagerModel* getServerManagerModel() const
    { return this->ServerManagerModel; }

  pq3DWidgetFactory* get3DWidgetFactory() const
    { return this->WidgetFactory; }

  /// pqLinksModel is the model used to keep track of proxy/property links
  /// maintained by vtkSMProxyManager.
  /// TODO: It may be worthwhile to investigate if we even need a global
  /// pqLinksModel. All the information is already available in
  /// vtkSMProxyManager.
  pqLinksModel* getLinksModel() const
    { return this->LinksModel; }

  /// pqPluginManager manages all functionality associated with loading plugins.
  pqPluginManager* getPluginManager() const
    { return this->PluginManager; }

  /// ProgressManager is the manager that streamlines progress.
  pqProgressManager* getProgressManager() const
    { return this->ProgressManager; }

  //// Returns the display policy instance used by the application.
  //// pqDisplayPolicy defines the policy for creating representations
  //// for sources.
  pqDisplayPolicy* getDisplayPolicy() const
    { return this->DisplayPolicy; }

  /// It is possible to change the display policy used by
  /// the application. Used to change the active display
  /// policy. The pqApplicationCore takes over the ownership of the display policy.
  void setDisplayPolicy(pqDisplayPolicy* dp);

  /// Returns the server manager selection model which keeps track of the active
  /// sources/filters.
  pqServerManagerSelectionModel* getSelectionModel()
    { return this->SelectionModel; }

  /// Provides access to the test utility.
  virtual pqTestUtility* testUtility();

  /// Set/Get the lookup table manager. Lookup table manager is used to manage
  /// lookup tables used for coloring using data arrays.
  /// policy. The pqApplicationCore takes over the ownership of the manager.
  void setLookupTableManager(pqLookupTableManager*);
  pqLookupTableManager* getLookupTableManager() const
    { return this->LookupTableManager; }

  /// Returns the manager for the global properties such as ForegroundColor etc.
  vtkSMGlobalPropertiesManager* getGlobalPropertiesManager();

  /// Returns the set of available server resources
  pqServerResources& serverResources();
  /// Set server resources
  void setServerResources(pqServerResources* serverResources);
  /// Returns an object that can start remote servers
  pqServerStartups& serverStartups();

  /// Get the application settings.
  pqSettings* settings();

  /// Save the ServerManager state.
  vtkPVXMLElement* saveState();
  void saveState(const QString& filename);

  /// Loads the ServerManager state. Emits the signal
  /// stateLoaded() on loading state successfully.
  void loadState(vtkPVXMLElement* root, pqServer* server);
  void loadState(const char* filename, pqServer* server);

  /// Check to see if its in the process of loading a state
  /// Reliance on this flag is chimerical since we cannot set this ivar when
  /// state file is  being loaded from python shell.
  bool isLoadingState(){return this->LoadingState;};

  /// Loads global properties values from settings.
  /// HACK: Need more graceful way of dealing with changes to settings and
  /// updating items that depend on it.
  void loadGlobalPropertiesFromSettings();

  /// loads palette i.e. global property values given the name of the palette.
  void loadPalette(const QString& name);

  /// loads palette i.e. global property values given the name XML state for a
  /// palette.
  void loadPalette(vtkPVXMLElement* xml);

  /// save the current palette as XML. A new reference is returned, so the
  /// caller is responsible for releasing memory i.e. call Delete() on the
  /// returned value.
  vtkPVXMLElement* getCurrrentPalette();

  /// returns the active server is any.
  pqServer* getActiveServer() const;

  /// Called to load the configuration xml bundled with the application the
  /// lists the plugins that the application is aware by default. If no filename
  /// is specified, {executable-path}/.plugins is loaded.
  void loadDistributedPlugins(const char* filename=0);

  /// Destructor.
  virtual ~pqApplicationCore();
public slots:
  /// Called QCoreApplication::quit().
  /// Applications should use this method instead of directly
  /// calling QCoreApplication::quit() since this ensures
  /// that any cleanup is performed correctly.
  void quit();

  /// Causes the output window to be shown.
  void showOutputWindow();

  /// Load configuration xml. This results in firing of the loadXML() signal
  /// which different components that support configuration catch and process to
  /// update their behavior.
  void loadConfiguration(const QString& filename);

  /// Renders all windows
  void render();

signals:
  // Fired when a state file is loaded successfully.
  // GUI components that may have state saved in the XML state file must listen
  // to this signal and handle process the XML to update their state.
  void stateLoaded(vtkPVXMLElement* root, vtkSMProxyLocator* locator);

  // Fired to save state xml. Components that need to save XML state should
  // listen to this signal and add their XML elements to the root. DO NOT MODIFY
  // THE ROOT besides adding new children.
  void stateSaved(vtkPVXMLElement* root);

  /// Fired when the undo stack is set.
  void undoStackChanged(pqUndoStack*);

  /// Fired on loadConfiguration().
  void loadXML(vtkPVXMLElement*);

protected slots:
  void onStateLoaded(vtkPVXMLElement* root, vtkSMProxyLocator* locator);
  void onStateSaved(vtkPVXMLElement* root);

protected:
  bool LoadingState;

  pqOutputWindow* OutputWindow;
  pqOutputWindowAdapter* OutputWindowAdapter;
  pqOptions* Options;

  pq3DWidgetFactory* WidgetFactory;
  pqDisplayPolicy* DisplayPolicy;
  pqLinksModel* LinksModel;
  pqLookupTableManager* LookupTableManager;
  pqObjectBuilder* ObjectBuilder;
  pqPluginManager* PluginManager;
  pqProgressManager* ProgressManager;
  pqServerManagerModel* ServerManagerModel;
  pqServerManagerObserver* ServerManagerObserver;
  pqServerManagerSelectionModel* SelectionModel;
  pqUndoStack* UndoStack;
  pqServerResources* ServerResources;
  pqServerStartups* ServerStartups;
  pqSettings* Settings;
  QPointer<pqTestUtility> TestUtility;

private:
  Q_DISABLE_COPY(pqApplicationCore)

  class pqInternals;
  pqInternals* Internal;
  static pqApplicationCore* Instance;
  void constructor();
  void createOutputWindow();
  bool FinalizeOnExit;
};

#endif

