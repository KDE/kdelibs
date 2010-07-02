
  {
    ModelLayoutChangeCommand layoutChangeCommand(&rootModel);
    layoutChangeCommand.setInitialTree({{ event.interpretString }}
    );
    QList<ModelLayoutChangeCommand::PersistentChange> changes;
    // TODO: generate code for changes.
    layoutChangeCommand.setChanges(changes);
    layoutChangeCommand.doCommand();
  }
