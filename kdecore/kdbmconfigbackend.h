#ifndef KCONFIGDBMBACKEND_H
#define KCONFIGDBMBACKEND_H "$Id$"

class  KConfigDBMBackEndPrivate;

/**
  * @short (Not yet in libkdecore) Backend to use dbm files as configuration files.
  */

class KConfigDBMBackEnd
	: public KConfigBackEnd
{
public:
  /**
   * Construct a configuration back end.
   *
   * @param _config Specifies the configuration object which values
   *        will be passed to as they are read, or from where values
   *        to be written to will be obtained from.
   * @param _fileName The name of the file in which config
   *        data is stored.  All registered configuration directories
   *        will be looked in in order of decreasing relevance.
   * @param _resType the resource type of the fileName specified, _if_
   *        it is not an absolute path (otherwise this parameter is ignored).
   * @param _useKDEGlobals If true, the user's system-wide kdeglobals file
   *        will be imported into the config object.  If false, only
   *        the filename specified will be dealt with.
   */
  KConfigDBMBackEnd(KConfigBase *_config, const QString &_fileName,
                    const char * _resType, bool _useKDEGlobals = true)
    : KConfigBackEnd(_config, _fileName, _resType, _useKDEGlobals) {}

  /**
   * Destructor.
   */
  virtual ~KConfigDBMBackEnd() {};

  /**
   * Parse all DBM configuration files for a config object.
   *
   * @returns Whether or not parsing was successful.
   */
  bool parseConfigFiles();

  /**
   * write configuration data to file(s).
   */
  virtual void sync(bool bMerge = true);

protected:

  /**
   * Parse one configuration file.
   *
   * @param rFile The configuration file to parse
   * @param pWriteBackMap If specified, points to a KEntryMap where
   *        the data read from the file should be stored, instead of
   *        inserting them directly into the configuration object.
   *        Use this area as a "scratchpad" when you need to know what is
   *        on disk but don't want to effect the configuration object.
   * @param bGlobal Specifies whether entries should be marked as
   *        belonging to the global KDE configuration file rather
   *        than the application-specific KDE configuration file(s).
   */
  void parseSingleConfigFile(void *rFile, KEntryMap *pWriteBackMap = 0L,
                             bool bGlobal = false);
  /**
   * Write configuration file back.
   *
   * @param filename The name of the file to write.
   * @param bGlobal Specifies whether to write only entries which
   *        are marked as belonging to the global KDE config file.
   *        If this is false, it skips those entries.
   * @param bMerge Specifies whether the old config file already
   *        on disk should be merged in with the data in memory.  If true,
   *        data is read off the disk and merged.  If false, the on-disk
   *        file is removed and only in-memory data is written out.
   * @return Whether some entries are left to be written to other
   *         files.
   */
  bool writeConfigFile(QString filename, bool bGlobal = false, bool bMerge = true);

  /**
   * Retrieve the state of the app-config object.
   *
   * @see KConfig::getConfigState
   */
  virtual KConfigBase::ConfigState getConfigState() const;

private:
  KConfigDBMBackEndPrivate *d;

};

#endif
