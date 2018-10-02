static const char jsAddDynamicScopeTextBox[] PROGMEM = {
  "<script defer>"
  // Create Dynamic TextBox
  "function CreateTextBox(number){"
  "var div = document.createElement('DIV');"
  "div.className = 'textboxScope';"
  "var textBox = document.createElement('input');"
  "textBox.type = 'text';"
  "textBox.name = 'scope'+number;"
  "textBox.className = 'wide';"
  "var delButton = document.createElement('input');"
  "delButton.className = 'button link';"
  "delButton.type = 'button';"
  "delButton.value = 'Del';"
  "delButton.setAttribute('onclick','RemoveTextBox(this)');"
  "div.appendChild(textBox);"
  "div.appendChild(delButton);"
  "return div;}"
  //Add TextBox
"function AddTextBox() {"
"  var container = document.getElementById('ScopesContainer');"
"  var element =  document.getElementById('LastScope');"
"  if (typeof(element) == 'undefined' || element == null){"
"    element = document.createElement('input');"
"    element.type = 'hidden';"
"    element.id = 'LastScope';"
"    element.value = 0;"
"    container.appendChild(element);"
"  }"
"  element.value = parseInt(element.value) + 1;"
"  var div =  CreateTextBox(parseInt(element.value));"
"  container.appendChild(div);"
"}"
//Remove TextBox
"function RemoveTextBox(div) {"
"  document.getElementById('ScopesContainer').removeChild(div.parentNode);"
"}"
"</script>"
};

void Debug_Scopes_Render_Scripts()
{
  TXBuffer += jsAddDynamicScopeTextBox;
}

void Debug_Scopes_Render_Post() {
  String edit = WebServer.arg(F("edit"));
  if(edit.length()>0){
    size_t argsNumber = WebServer.args();
    activeDebugScopes.clear();
    for (size_t i = 0; i < argsNumber; i++) {
      if(WebServer.argName(i).startsWith(F("scope"))){
        String value = WebServer.arg(i);
        if(value.length()>0)
          activeDebugScopes.push_back(value);
      }
    }
  }
}

void Debug_Scopes_Render(/* arguments */) {
  //Add debug scopes
  TXBuffer+=F("<TR><TD valign = 'top'>");
  TXBuffer+=F("Log Scopes:");
  TXBuffer+=F("</TD><TD>");
  //Add scopes container
  TXBuffer += F("<div id='ScopesContainer'>");  // Start Scopes container

  TXBuffer += F("<input type='hidden' id='LastScope' value='"); // Add hidden field
  TXBuffer += activeDebugScopes.size();
  TXBuffer += F("'/>");

  int scopeIndex = 0;
  for (auto scope : activeDebugScopes){
      Debug_Scopes_Render_AddTexbox(scopeIndex++ , scope);
  }
  TXBuffer += F("</div>");                      // End Scopes container

  //add addButton
  TXBuffer += F("<input class='button link' type='button' onclick='AddTextBox()' value='add'/>");

  TXBuffer += F("</TD></TR>");                  // Close Debug scopes

}

void Debug_Scopes_Render_AddTexbox(int id, String& value) {
  TXBuffer += F("<div class='textboxScope'>");
  //Add TextBox
  TXBuffer += F("<input type=text class='wide' name=scope");
  TXBuffer += id;
  TXBuffer += F(" value='");
  TXBuffer += value;
  TXBuffer += F("'/>");
  //AddRemove Button
  TXBuffer += F("<input class='button link' type='button' value='Del' onclick = 'RemoveTextBox(this)'/>");
  TXBuffer += F("</div>");
}
