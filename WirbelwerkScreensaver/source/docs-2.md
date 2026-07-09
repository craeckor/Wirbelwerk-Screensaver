---
layout: Conceptual
title: Screensaver Sample | Microsoft Learn
canonicalUrl: https://learn.microsoft.com/en-us/previous-versions/windows/desktop/ms686421(v=vs.85)
breadcrumb_path: /previous-versions/windows/breadcrumb/toc.json
ROBOTS: INDEX,FOLLOW
is_archived: true
uhfHeaderId: MSDocsHeader-Archive
ms.author: Archiveddocs
ms.prod: windows-desktop
ms.topic: archived
ms.date: 2014-04-30T00:00:00.0000000Z
TOCTitle: Screensaver Sample
ms:assetid: VS|feedsapi|~\rss\howto\samp_screensaver.htm
ms:mtpsurl: https://msdn.microsoft.com/en-us/library/ms686421(v=VS.85)
ms:contentKeyID: 5563509
mtps_version: v=VS.85
api_name: 
topic_type:
- kbHowTo
api_type: 
api_location: 
locale: en-us
document_id: 5053fea6-8235-be25-64fc-66400ce53c13
document_version_independent_id: 5053fea6-8235-be25-64fc-66400ce53c13
updated_at: 2021-10-26T08:54:00.0000000Z
original_content_git_url: https://docs-archive.visualstudio.com/DefaultCollection/docs-archive-project/_git/windows-dev-docs-archive-desktop-pr?path=/windows-desktop/desktop/ms686421(v=vs.85).md&version=GBlive&_a=contents
gitcommit: https://docs-archive.visualstudio.com/DefaultCollection/docs-archive-project/_git/windows-dev-docs-archive-desktop-pr/commit/f3ce61e623570c4520cf86c82c52a4b6b0ce66b0?path=/windows-desktop/desktop/ms686421(v=vs.85).md&_a=contents
git_commit_id: f3ce61e623570c4520cf86c82c52a4b6b0ce66b0
site_name: Docs
depot_name: MSDN.windows-desktop-archive
page_type: conceptual
toc_rel: toc.json
feedback_system: None
feedback_product_url: ''
feedback_help_link_type: ''
feedback_help_link_url: ''
search.mshattr.devlang: jscript scr
word_count: 3378
asset_id: desktop/ms686421(v=vs.85)
moniker_range_name: 
monikers: []
item_type: Content
source_path: windows-desktop/desktop/ms686421(v=vs.85).md
platformId: 4b5387b8-b2df-f082-206d-17031cc357f2
---

# Screensaver Sample | Microsoft Learn

Screensaver Sample

The Screensaver sample is a complete application that demonstrates how to search the Common Feed List for Really Simple Syndication (RSS) feed items that contain images as enclosures, and display those images on the screen along with the headline and text of the item. Based on the Windows screensaver that is available as part of the Microsoft Visual C# Starter Kit, the sample incorporates the functionality of the Windows RSS Platform. The project comes ready to compile and run. You can customize the application and share your work with others.

Download the code: [Windows RSS Platform Sample Screensaver](https://www.microsoft.com/downloads/details.aspx?familyid=441cfa71-630d-49ae-80c4-fed49163afae&amp;displaylang=en).

This topic contains the following sections.

- Getting Started
    - System Requirements
    - Building and Running the Sample
    - How to Create an .SCR File
    - Using the Screensaver from Windows
    - Interacting with the Screensaver
- Project Files
- Concepts
    - Command Line Options
    - Enumerating Feeds
    - Timer Events
- Expanding the Screensaver
    - Download Missing Enclosures
    - Open Current Item in Browser
    - More Suggestions

## Getting Started

When the screensaver is launched, it scans the Common Feed List for RSS feed items that contain enclosures (binary attachments) that can be displayed. Then it creates a full-screen Windows Form and displays data from the feeds. A timer is used to update the currently selected topic and change the background image.

If You move or click the mouse at any point or press a key, the program exits immediately.

This section contains the following subsections:

- System Requirements
- Building and Running the Sample
- How to Create an .SCR File
- Using the Screensaver from Windows
- Interacting with the Screensaver

### System Requirements

The Windows RSS Platform Screensaver sample project requires Visual C# 2005 and uses version 2.0 of the Microsoft .NET Framework.

The Framework is installed as part of Microsoft Visual Studio, but it can also be installed directly from the [Microsoft .NET Framework Developer Center](https://msdn.microsoft.com/en-us/netframework/aa569263.aspx). Any computer that runs the application must have version 2.0 of the Framework installed. If you plan to share your screensaver with friends, you should let them know that they must also install Framework version 2.0.

### Building and Running the Sample

After your project is loaded into the Visual C# 2005 environment, you can compile and run it in one step.

| To build and run the screensaver application: |
| --- |
| 1. Press **F5** to build and start the screensaver under the debugger. The screensaver launches and starts displaying images from feed items in the Common Feed List.<br>2. To exit the screensaver, press a key or move the mouse. |

*If the screensaver does not build*, ensure that the latest release of the `Feeds` reference is loaded. Also note that the capitalization of some property names has been changed for the final release of the Windows RSS Platform, such as IFeed.Name and IFeed.Path.

| To add a reference to the interop assembly: |
| --- |
| 1. In the **Solution Explorer**, expand the list of References.<br>2. If it exists, right-click `Microsoft.Feeds.Interop` and select **Remove Reference** from the context menu.<br>3. Right-click `References` and select **Add Reference**.![Click &#39;Add Reference&#39;](images/ms686421.ss_addref(en-us,vs.85).gif)<br>4. Click the **COM** tab of the **Add Reference** dialog. Locate "Microsoft Feeds, version 1.0" in the list.![Add Reference Dialog](images/ms686421.ss_addref2(en-us,vs.85).gif)<br>5. Select it and click **OK**. |

### How to Create an .SCR File

The ScreenSaver project file has been extended with a custom build task that renames the output of the build (an executable file) with the .SCR extension. The following lines in ScreenSaver.csproj perform the copy as an "AfterBuild" step.

```
<Import Project="$(MSBuildBinPath)\Microsoft.CSharp.targets" />
<Target Name="AfterBuild">
  <Copy SourceFiles="$(TargetDir)\$(TargetName).exe" DestinationFiles="$(ProjectDir)\bin\ssNews.scr"
        SkipUnchangedFiles="true" />
</Target> 
```

See [How to: Extend the Visual Studio Build Process](https://msdn.microsoft.com/en-us/library/ms366724%28v=vs.85%29) for more information.

### Using the Screensaver from Windows

After you have built the application, you can install it as a Windows screensaver.

| To "install" the screensaver by hand: |
| --- |
| 1. Open the RssScreenSaver project directory with Windows Explorer, then open the `bin` subfolder.<br>2. Copy `ssNews.scr` to your Windows directory. For both Windows XP and Windows Vista this folder is \Windows\System32 on your system drive.<br>3. Open the `bin\Release` project subfolder, and locate `Interop.Microsoft.Feeds.Interop.dll`. Copy this file to the same location as `ssNews.scr` in the step above.<br>4. Finally, set "News" as the active Windows screensaver. (See steps below.) |

Alternatively, you can specify a post-build step that performs the copy for you.

| To copy the output files as part of the build: |
| --- |
| 1. On the **Project** menu in Visual Studio 2005, select **ScreenSaver Properties...**<br>2. Select the Build Events tab, and enter the following in the "Post-build event command line" text box:<br><br><br>    ```<br>    copy "$(TargetPath)" %SystemRoot%\system32\ssNews.scr<br>    copy "$(TargetDir)Interop.Microsoft.Feeds.Interop.dll" %SystemRoot%\system32<br>    ```<br>3. Save the changes, and rebuild the project. |

To complete the installation, you must set the RSS screensaver as the default screensaver for Windows.

| To set the active screensaver for Windows: |
| --- |
| 1. Right-click the Desktop and select **Properties** (Windows XP) or **Personalize** (Windows Vista) from the context menu.<br>2. Click the **Screen Saver** tab.<br>3. Click the **Screen saver** drop-down list, and select `News`.<br>4. Click the **Settings** button to select screensaver options. Click **OK** when finished.<br>5. Click the **Preview** button to test the screensaver. Move the mouse or press any key to stop the screensaver.<br>6. Click **OK** to close the dialog box and save your settings. |

You have now installed the screensaver. When Windows activates the screensaver due to a period of inactivity, it launches the **ssNews.scr** executable.

### Interacting with the Screensaver

While the screensaver is active, you can use keyboard shortcuts to view the loaded list of feeds and items, and move forward and backward through the images.

| Key | Description |
| --- | --- |
| Alt+F | View Feed List |
| Alt+I | View Item List |
| Up Arrow | Move to Previous Item |
| Down Arrow | Move to Next Item |

## Project Files

The project contains the following source files:

| Program.cs | Contains the **Main()** method–the location where the program begins execution–and the code for handling the command line switches. |
| --- | --- |
| ScreenSaverForm.cs | Contains the main screensaver Windows Form. The images and feed item headlines are displayed on this full-screen form. |
| OptionsForm.cs | Contains the Windows Form that displays the settings options. From this dialog box, you can configure the behavior of the screensaver. |
| CommonFeedListUtils.cs | Contains the **CommonFeedList** class, which is used to recursively enumerate all the feeds in a base folder. |
| FeedList.cs | Contains the active list of **RssFeed** objects. This class collects items with image enclosures and fetches the next and previous items in the list. |
| RssFeed.cs | Contains the class representing the RSS feed. The feed can be loaded from XML text, an XML node, a Uniform Resource Identifier (URI), or from an [IFeed](ms686325%28v=vs.85%29) object. |
| RssItem.cs | Contains the class representing the RSS feed item. For the purposes of the sample, the class will not load the item if it does not contain an enclosure of the expected type (.JPG, .GIF, .PNG, or .BMP). |
| IItem.cs | Contains the class representing an item–an object with a title and a description. This object is rendered using the **ItemListView** and **ItemDescriptionView** types. |
| ItemDescriptionView.cs | Contains code that displays the description of an item from an RSS feed. It slowly fades in the text of the item, and then fades it out again. |
| ItemListView.cs | Contains code that displays the title of an item from an RSS feed. It keeps track of a currently selected article, which it can scroll forward or backward. |
| FeedListView.cs | Contains code that displays the list of RSS feed titles. |

The project also contains the following resource files:

| App.ico | The screensaver application icon. |
| --- | --- |
| DefaultRss.xml | Contains the text of a default feed, used if no suitable feed enclosures can be found. |
| SSaverBackground.jpg | A built-in image defined by the default feed. |
| SSaverBackground2.jpg | Another built-in image defined by the default feed. |

## Concepts

This section contains the following subsections:

- Command Line Options
- Enumerating Feeds
- Timer Events

### Command Line Options

When Windows runs your screensaver, it launches it with one of three command line options:

- `/s` – Start the screensaver in full-screen mode.
- `/c` – Show the configuration settings dialog box.
- `/p ####` – Display a preview of the screensaver using the specified window handle.

The screensaver sample handles these arguments in the **Program.cs** source file; however, the `/p` argument is not implemented.

In addition to these arguments, the sample also supports the following option:

- `/d` – Start the screensaver in debug mode.

The debug mode flag is passed as a command line parameter when debugging in Visual Studio. In debug mode, the feed and item list are displayed by default. In addition, the form reverts to a standard sizeable border style to simplify the debugging process.

### Enumerating Feeds

To simplify the process of walking the hierarchy of the feedlist, **CommonFeedListUtils.cs** declares a helper class to keep track of the progress of the enumeration. It relies on a mechanism that is new to version 2.0 of the Framework, the **yield** keyword, to return each feed object sequentially to the calling process. Internally, the method uses the [Queue](https://msdn.microsoft.com/en-us/library/055sfc3z%28v=vs.85%29) class to manage a list of folders. The method takes a single parameter, the folder object, to use as the base of the enumeration.

```
public static IEnumerable<IFeed> CommonFeedList(IFeedFolder folder)
{
    Queue<IFeedFolder> queue = new Queue<IFeedFolder>();
    queue.Enqueue(folder);
    while (queue.Count > 0)
    {
        IFeedFolder currentFolder = queue.Dequeue();
        foreach (IFeedFolder subfolder in (IFeedsEnum)currentFolder.Subfolders)
            queue.Enqueue(subfolder);

        foreach (IFeed feed in (IFeedsEnum)currentFolder.Feeds)
            yield return feed;
    }
}
```

The *CommonFeedList()* method is used to load the list of feeds in **FeedList.cs**. The method is designed to be used in a **foreach** loop, as follows:

```
IFeedsManager fs = new FeedsManagerClass();

foreach (IFeed feed in CommonFeedListUtils.CommonFeedList((IFeedFolder)fs.RootFolder))
{
    // Using System.Diagnostics for Debug class...
    Debug.Print("Found feed {0} with {1} items.", feed.name, feed.ItemCount);

    //*** Do work here
}
```

### Timer Events

Two System.Windows.Forms.Timer objects produce the events that keep the screensaver in motion.

- **checkCflTimer** – Before the screensaver starts, it loads a list of feed items that contain attached images from the Common Feed List. While the screensaver is running, timer events from **checkCflTimer** cause the screensaver to refresh the internal list of items based on the [LastWriteTime](ms684726%28v=vs.85%29) property of the feed. This timer is visible on the **ScreenSaverForm** in design mode.
- **fadeTimer** – Another timer is created in the constructor of **ItemDescriptionView** that is used to fade the item description in and out, and to move to the next item in the list. There are two event handlers for this timer, one in **ItemDescriptionView.cs**, the other in **ScreenSaverForm.cs**.

By default, the interval of **fadeTimer** is set to 40 msecs. At each tick event, two things happen: the opacity level of the **ItemDescriptionView** is adjusted by the value of *textAlphaDelta*, and the **ScreenSaverForm** is refreshed and repainted. Two "terminal" effects also occur when the opacity reaches its maximum and minimum values.

First, at maximum opacity, the Interval property of **fadeTimer** is changed to the value of *fadePauseInterval*, which is 2000 msecs by default. During this pause, the screen will not refresh automatically, so you must explicitly call *Refresh()* in response to events that update the screen, such as **KeyDown** events. You can modify the length of the pause in the Configuration dialog box.

Next, at minimum opacity, the *fadeTimer\_Tick()* event handler triggers a secondary event of the view called **FadingComplete**. The event handler for this event moves the current index of the **FeedList** to the next item.

## Expanding the Screensaver

You are invited to expand the screensaver using the following features or create other features.

- Download Missing Enclosures
- Open Current Item in Browser
- More Suggestions

### Download Missing Enclosures

If Windows Internet Explorer doesn't download enclosures automatically, how does the screensaver find images to display? The answer is: it doesn't. If an image has not been downloaded, the screensaver displays a blank screen. Even if you have clicked on the link in the browser and the image has been downloaded to the Temporary Internet Files (TIF) folder; the screensaver can only discover and display files that have been downloaded to the enclosures folder associated with the feed.

To fix this problem, select the 'Automatically download enclosures' option when you subscribe to a picture feed. This option can be enabled in the **Properties** dialog box for the feed. However, you can also do it programmatically through the Windows RSS Platform.

The sample currently displays enclosures based on their file name extensions. The **FeedList.cs** file contains a declaration of a list of extensions for this purpose.

```
internal readonly static List<string> imageExtensions = new List<string>(
   new string[] { "*.BMP", "*.GIF", "*.PNG", "*.JPG", "*.JPEG" });
```

In the **RssItem.cs** file, the file name extension of the downloaded enclosure is compared to the list of file name extensions, and the object is finalized if a match is found.

```
   // Let's make sure we only add pictures
   string filename = GetLocalPath(enclosure);
   string extension = "*" + Path.GetExtension(filename);
   extension = extension.ToUpperInvariant();
   if (FeedList.imageExtensions.Contains(extension))
      this.enclosure = new FileInfo(filename);
```

Checking the file name extension works only for enclosures that have already been downloaded. If the file doesn't exist, you must to check the MIME type of the enclosure. You can also parse the file name extension of the URL. The URL may include query parameters, or refer to a file type different from the file type it returns, for example, PHP or ASPX.

First, add a new list of MIME types associated with images.

```
internal readonly static List<string> imageTypes = new List<string>(
   new string[] { "image/gif", "image/jpeg", "image/jpg", "image/bmp", 
                  "image/x-ms-bmp", "image/png", "image/x-png" });
```

Next, add the following code to **RssItem.cs**.

```
   // Verify that the enclosure has been downloaded
   if (enclosure.DownloadStatus != FEEDS_DOWNLOAD_STATUS.FDS_DOWNLOADED)
   {
      // Only download images!! Check MIME type for image first...
      if (FeedList.imageTypes.Contains(enclosure.Type))
      {
         if (enclosure.DownloadStatus == FEEDS_DOWNLOAD_STATUS.FDS_NONE)
            enclosure.AsyncDownload();
      }
      return;
   } 
```

Verify the extension of the file after it has been downloaded to ensure that it is the file type that you expected to receive.

### Open Current Item in Browser

Now that the screensaver is working, you might notice that only a portion of the item's descriptive text can be displayed in the main window. If your feeds have been updated recently, you may discover items that you have not seen before. After the item is in view, you can open the browser and view the full article, or view other items from the same feed.

To open the current item in the browser, do the following:

1. Locate the URL of the Current Item.
2. Launch the Browser.
3. Add a Keyboard Shortcut.
4. Start a New Thread.
5. Secure a Locked Workstation.

#### Locate the URL of the Current Item.

To open the current item in the browser, you must use the URL of the item. The [Link](aa359069%28v=vs.85%29) property of the feed item is copied to the *RssItem* objects as they are constructed. The **FeedList** class, which maintains the list of feeds and the current item in the feed, can be used to find the URL.

The following code creates a new member function of the **FeedList** class that retrieves the URL from the current item.

```
public void OpenItem()
{
    RssItem item = CurrentItem;
    Debug.Print("Opening item '{0}' from '{1}'", item.Description, item.Link);

    // More code to follow...
}
```

Some feeds use a relative URL for the item link. You can create an absolute URI from the base URL of the current feed and `item.Link` using the System.Uri constructor to combine the two. If the item link is not a relative URL, the base value is ignored.

```
    // Construct an absolute Uri from base and relative path
    Uri ItemUri = new Uri(new Uri(CurrentFeed.Link), item.Link);
```

#### Launch the Browser.

You can launch Internet Explorer 7 with the retrieved URL as a command line argument. Use the **ProcessStartInfo** class to declare the elements used to start the new process. The following code assumes you have imported the types defined in the Systems.Diagnostics namespace with the **using** directive.

```
    // Launch the browser
    ProcessStartInfo si = new ProcessStartInfo();
    si.UseShellExecute = true;
    si.FileName = "iexplore.exe";
    si.Arguments = ItemUri.ToString();
    Process proc = Process.Start(si);
```

#### Add a Keyboard Shortcut.

To expose the new functionality from the interface, add a call to *OpenItem()* to the *ScreenSaverForm\_KeyDown* event handler in **ScreenSaverForm.cs**. The following code uses the **Enter** key, but you can choose a different key. Make sure the new code is added after the call to *Close()* because the form must exit so that you can access the browser window.

```
// Allow close, and launch browser prior to exit
if (e.KeyCode == Keys.Enter)
    feedlist.OpenItem();
```

Take a moment to test the new feature. Notice that the browser appears as expected if you launch the screensaver from the debugger; however, nothing seems to happen if the screensaver is launched from the operating system.

##### Start a New Thread.

The difference in behavior results from the process that is used to launch the screensaver. When the main screensaver thread exits, the operating system stops any child processes that are still running. When the screensaver is in the Visual Studio debugger, the Internet Explorer process is not linked to the screensaver as a child process. However, when the screensaver is launched from the Windows Logon Process (Winlogon.exe), the child process has barely enough time to start before it is asked to shut down.

The solution to this dilemma is a new thread. To prevent the screensaver application from exiting until the browser window is closed, call *WaitForExit()* on the newly created process to wait for the child process to end before continuing. You must close the screensaver form so that the browser is visible.

There are two parts to this procedure. First, create a new class to encapsulate the functionality to run in a new thread, as in the following example:

```
public class ExplorerThread
{
    public void OpenItem(Object LinkUrl)
    {
        ProcessStartInfo si = new ProcessStartInfo();
        si.UseShellExecute = true;
        si.FileName = "iexplore.exe";
        si.Arguments = (string)LinkUrl;
        Process proc = Process.Start(si);
        if (null != proc)
            proc.WaitForExit();   // Block until exit
    }
}
```

Next, start the new thread from the *OpenItem()* method created above. The following code passes the URL to the thread using a **ParameterizedThreadStart** class, available in version 2.0 of the Framework.

```
public void OpenItem()
{
    RssItem item = CurrentItem;
    Debug.Print("Opening item '{0}' from '{1}'", item.Description, item.Link);

    // Construct an absolute Uri from base and relative path
    Uri ItemUri = new Uri(new Uri(CurrentFeed.Link), item.Link);

    // Thread will block exit until child process exits
    ExplorerThread ext = new ExplorerThread();
    Thread t = new Thread(new ParameterizedThreadStart(ext.OpenItem));
    t.Start(ItemUri.ToString());
}
```

Test the feature again. As long as the browser window remains open, there is at least one active thread running in the application, which prevents it from exiting when the main **ScreenSaverForm** is closed.

##### Secure a Locked Workstation.

For many users, the screensaver is a means of securing the desktop from intruders. If the workstation is locked, the screensaver must not allow access to the system unless a password is entered. Unfortunately, implementing the preceding code opens a very large security hole. When the user presses the **Enter** key, the browser appears whether the desktop is locked or not. With the browser open, the file system and desktop are readily accessible.

Before a screensaver appears, Windows creates a new virtual desktop for it. The screensaver process is created on this second desktop, which is never locked. Any other processes that the screensaver creates will inherit this virtual desktop, unless the application switches back to the default desktop. To successfully switch back to the default desktop, certain access rights must be granted. If the switch fails, the workstation is most likely locked.

The following code implements this functionality.

```
using System.Runtime.InteropServices;

public static class Desktop
{
    private const uint DESKTOP_READOBJECTS = 0x0001;
    private const uint DESKTOP_WRITEOBJECTS = 0x0080;
    private const uint DESKTOP_SWITCHDESKTOP = 0x0100;
    private const uint AccessUnlocked = DESKTOP_READOBJECTS | 
            DESKTOP_WRITEOBJECTS | DESKTOP_SWITCHDESKTOP;

    [DllImport("user32.dll", CharSet=CharSet.Auto, SetLastError=true)]
    private static extern IntPtr OpenDesktop(string lpszDesktop, int dwFlags, 
            bool fInherit, uint dwDesiredAccess);

    [DllImport("user32.dll")]
    private static extern bool SwitchDesktop(IntPtr hDesktop);

    public static bool IsNotLocked()
    {
        IntPtr hdt = OpenDesktop("Default", 0, false, AccessUnlocked);

        // If we can make the switch, the desktop is not locked
        if (SwitchDesktop(hdt))
            return true;

        return false;
    }
}
```

To complete this step, verify that *Desktop.IsNotLocked()* is true before you create the browser thread.

### More Suggestions

The screensaver application is designed to be easily expanded and customized. Some ideas for extending it are:

- **Randomly select the item to display**: Mix up the images to keep things interesting.
- **Respond to feed and folder events**: Rewrite the polling feature to listen for [FeedDownloadCompleted](ms684769%28v=vs.85%29) or [FeedItemCountChanged](ms684771%28v=vs.85%29) events, and refresh the *FeedList* only when necessary.
- **Set the desktop wallpaper**: Add a keyboard shortcut to set the current image as the background image of the desktop. Use [SystemParametersInfo](https://msdn.microsoft.com/en-us/library/cc429946%28v=vs.85%29) with `SPI_SETDESKWALLPAPER` to specify the image file.
- **Transition between images**: Create mesmerizing visual effects with Microsoft DirectX Image Transformations.
- **Render HTML markup in the item description**: Host the Internet Explorer WebBrowser Control to do the work.
- **Show weather information**: Display current local weather conditions from a syndicated weather service.