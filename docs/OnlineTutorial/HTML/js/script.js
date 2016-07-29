// Index of pages
//         FileName                     Title
pages = [["main",                       "Getting Started with Repast HPC"],
         ["TOC",                        "Table of Contents"],
         ["HowToUse",                   "How to use this tutorial"],
         ["RepastHPC_Demos",            "Repast HPC Demos in this Tutorial"], 
         ["RepastHPC_Demo_00_Overview", "HPC Demo 00 Overview"],
         ["RepastHPC_Demo_00_Step_01",  "HPC:D00, Step 01: Hello, World!"],
         ["RepastHPC_Demo_00_Step_02",  "HPC:D00, Step 02: A (very) little MPI"],
         ["RepastHPC_Demo_00_Step_03",  "HPC:D00, Step 03: A little more MPI"],
         ["RepastHPC_Demo_00_Step_04",  "HPC:D00, Step 04: Giving MPI a Boost"],
         ["RepastHPC_Demo_00_Step_05",  "HPC:D00, Step 05: Repast Process"],
         ["RepastHPC_Demo_00_Step_06",  "HPC:D00, Step 06: A First Repast HPC Model"],
         ["RepastHPC_Demo_00_Step_07",  "HPC:D00, Step 07: Initializing a Schedule"],
         ["RepastHPC_Demo_00_Step_08",  "HPC:D00, Step 08: Scheduling Events"],
         ["RepastHPC_Demo_00_Step_09",  "HPC:D00, Step 09: Recurring Events"],
         ["RepastHPC_Demo_00_Step_10",  "HPC:D00, Step 10: A Complex Schedule"],
         ["RepastHPC_Demo_00_Step_11",  "HPC:D00, Step 11: Model Properties Files"],
         ["RepastHPC_Demo_00_Step_12",  "HPC:D00, Step 12: Scalable Properties"],
         ["RepastHPC_Demo_00_Step_13",  "HPC:D00, Step 13: Command Line Properties"],
         ["RepastHPC_Demo_00_Step_14",  "HPC:D00, Step 14: Writing Properties to Files"],
         ["RepastHPC_Demo_00_Step_15",  "HPC:D00, Step 15: End Events/Writing Properties II"],
         ["RepastHPC_Demo_01_Overview", "HPC Demo 01 Overview"],
         ["RepastHPC_Demo_01_Step_00",  "HPC:D01, Step 00: Directories and Files"],
         ["RepastHPC_Demo_01_Step_01",  "HPC:D01, Step 01: Agents"],
         ["RepastHPC_Demo_01_Step_02",  "HPC:D01, Step 02: Context is Everything"],
         ["RepastHPC_Demo_01_Step_03",  "HPC:D01, Step 03: Initialization and Agent Creation"],
         ["RepastHPC_Demo_01_Step_04",  "HPC:D01, Step 04: Randomness and Reproducibility"],
         ["RepastHPC_Demo_01_Step_05",  "HPC:D01, Step 05: Agent Scheduling"],
         ["RepastHPC_Demo_01_Step_06",  "HPC:D01, Step 06: Agents Acting"],
         ["RepastHPC_Demo_01_Step_07",  "HPC:D01, Step 07: Agent Packages"],
         ["RepastHPC_Demo_01_Step_08",  "HPC:D01, Step 08: Sending Agent Packages"],
         ["RepastHPC_Demo_01_Step_09",  "HPC:D01, Step 09: Sharing Agents Across Processes"],
         ["RepastHPC_Demo_01_Step_10",  "HPC:D01, Step 10: Thinking Globally but Acting Locally"],
         ["RepastHPC_Demo_01_Step_11",  "HPC:D01, Step 11: Updating (Synchronizing) Non-Local Agents"],
         ["RepastHPC_Demo_01_Step_12",  "HPC:D01, Step 12: Canceling Agent Sharing"],
         ["RepastHPC_Demo_01_Step_13",  "HPC:D01, Step 13: Removing Imported Agents"],
         ["RepastHPC_Demo_01_Step_14",  "HPC:D01, Step 14: Removing Local Agents"],
         ["RepastHPC_Demo_01_Step_15",  "HPC:D01, Step 15: More Magic: Moving Agents Across Processes"],
         ["RepastHPC_Demo_01_Step_16",  "HPC:D01, Step 16: Agent Requests in sets"],
         ["RepastHPC_Demo_01_Step_17",  "HPC:D01, Step 17: Collecting data"],
         ["RepastHPC_Demo_02_Overview", "HPC Demo 02 Overview"],
         ["RepastHPC_Demo_02_Step_00",  "HPC:D02, Step 00: Initial code"],
         ["RepastHPC_Demo_02_Step_01",  "HPC:D02, Step 01: Creating the network projection"],
         ["RepastHPC_Demo_02_Step_02",  "HPC:D02, Step 02: Making network connections"],
         ["RepastHPC_Demo_02_Step_03",  "HPC:D02, Step 03: Using network connections"],
         ["RepastHPC_Demo_02_Step_04",  "HPC:D02, Step 04: Weighted network edges"],
         ["RepastHPC_Demo_02_Step_05",  "HPC:D02, Step 05: Custom Edges"],
         ["RepastHPC_Demo_03_Overview", "HPC Demo 03 Overview"],
         ["RepastHPC_Demo_03_Step_00",  "HPC:D03, Step 00: Initial code"],
         ["RepastHPC_Demo_03_Step_01",  "HPC:D03, Step 01: Creating the spatial projection"],
         ["RepastHPC_Demo_03_Step_02",  "HPC:D03, Step 02: Agents Moving in Space"],
         ["RepastHPC_Demo_03_Step_03",  "HPC:D03, Step 03: Finding agents near each other: Querying spaces"],
         ["RepastHPC_Demo_03_Step_04",  "HPC:D03, Step 04: Strict Borders"],
         ["RepastHPC_Demo_03_Step_05",  "HPC:D03, Step 05: Continuous Spaces"],
         ["RepastHPC_Demo_03_Step_06",  "HPC:D03, Step 06: Multiple spatial projections: coterminous"],
         ["RepastHPC_Demo_03_Step_07",  "HPC:D03, Step 07: Multiple spatial projections: non-coterminous"],
         ["RepastHPC_Demo_03_Step_08",  "HPC:D03, Step 08: Spatial and network projections in one simulation"],
         ["RepastHPC_Demo_04_Overview", "HPC Demo 04 Overview"],
         ["RepastHPC_Demo_04_Step_00",  "HPC:D04, Step 00: Initial code"],
         ["RepastHPC_Demo_04_Step_01",  "HPC:D04, Step 01: Creating N-Dimensional Spaces"],
         ["RepastHPC_Demo_04_Step_02",  "HPC:D04, Step 02: Creating N-Dimensional Value Layers"],
         ["RepastHPC_Demo_04_Step_03",  "HPC:D04, Step 03: Synchronization of Values Layers vs. Projections"],
         ["RepastHPC_Demo_04_Step_04",  "HPC:D04, Step 04: Using N-Dimensional Value Layers"],
         ["RepastHPC_Demo_04_Step_05",  "HPC:D04, Step 05: Synchronous Updates in Value Layers"],
         ["RepastHPC_Demo_04_Step_06",  "HPC:D04, Step 06: Diffusion on N-Dimensional Value Layers"]
];         

function pageIndex(pageName){
  var i = 0;
  while(i < pages.length){
    if(pages[i][0] == pageName) return i;
    i++;
  }
  return -2;
}

function linkToPage(pageIndex){
  if(pageIndex < 0)             return "";
  if(pageIndex >= pages.length) return "";
  return "<a href=\"" + pages[pageIndex][0] + ".html\">" + pages[pageIndex][1] + "</a>";
}

function getLinkToPage(pageName){
  document.write(linkToPage(pageIndex(pageName)));
}

function toc(){
  document.writeln("<ul>");
  var i = 0;
  while(i < pages.length){
    document.writeln("  <li>" + linkToPage(i) + "</li>"); 
    i++;
  }
  document.writeln("</ul>");
}

function header(pageName){
  var i = pageIndex(pageName);
  document.write("<hr/><hr/><center><h1 class=\"header\">RepastHPC Tutorial</h1></center></h1><h2><center>" + pages[i][1] + "</center></h2><hr/>");
  var i = pageIndex(pageName);
  var prev = i - 1;
  var next = i + 1;
  document.write("<table width=100%><td width=33% class=\"left\">Prev: " + linkToPage(prev) + "</td><td width=33%><center>" + linkToPage(pageIndex("TOC")) + "</center></td><td width=\"33%\" class=\"right\">Next: " + linkToPage(next) + "</td></tr></table><hr/>");
}

function footer(pageName){
  var i = pageIndex(pageName);
  var prev = i - 1;
  var next = i + 1;
  document.write("<hr/><table width=100% class=left><td width=33%>Prev: " + linkToPage(prev) + "</td><td width=33%><center>" + linkToPage(pageIndex("TOC")) + "</center></td><td width=33% class=right>Next: " + linkToPage(next) + "</td></tr><tr><td width=100% colspan=3><center>RepastHPC Tutorial</center></td></tr></table><hr/>");
}

function toggleSidebar(id){
  var item = document.getElementById(id + "_DEF")
  item.className= (item.className=='defHIDDEN') ? 'defSHOW' : 'defHIDDEN';
}

