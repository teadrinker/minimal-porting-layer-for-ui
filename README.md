## minimal-porting-layer-for-ui


***! ! ! WARNING, this is code old ! ! !***


    This is a minimal OS abstraction for custom, software-rendered UI:s
    This was designed to be as thin as possible for porting (vsts),
    it's not supposed to be used directly from application code...

    Also, you probably don't want to use modal loops etc like it's the 90s...

    This is only the windows backend (osx carbon is long since dead, and the
    cocoa impl is no longer in sync with the interface in the header file)
        
    By Martin Eklund ca 2004-2006
    Drag'n drop code based on tutorial by James Brown (www.catch22.net)
    
