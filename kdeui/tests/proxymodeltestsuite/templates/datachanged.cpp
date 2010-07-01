
  {
    ModelDataChangeCommand dataChangeCommand(&rootModel);
    dataChangeCommand.setAncestorRows({{ event.rowAncestors }});
    dataChangeCommand.setStart({{ event.start }});
    dataChangeCommand.setEnd({{ event.end }});
    dataChangeCommand.doCommand();
  }