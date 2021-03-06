// Dropdown menu for selecting entity type
var entityTypeSelect = '<select><option value="1">'+getEntityTypeString(1)+'</option><option value="2">'+getEntityTypeString(2)+'</option></select>'

// Multiselect menu for selecting operation
var operationSelect = '<select multiple><option value="1">Retrieve</option><option value="2">Sign</option><option value="4">Verify</option><option value="8">Encrypt</option><option value="16">Decrypt</option><option value="32">HMAC</option></select>'

/*
 * Utility function to map entity types numbers to values.
 * See db_types.h
 */
function getEntityTypeString(typeNumber)
{
	switch(typeNumber)
	{
	case 1:
		return 'User';
	case 2:
		return 'POSIX Group';
	}
}

/*
 * Utility function to map operation number to string.
 * See db_types.h
 */
function getOpString(opNumber)
{
	var arr = [];

	if (opNumber & 1)
	{
		arr.push("Retrieve");
	}
	if (opNumber & 2)
	{
		arr.push("Sign");
	}	
	if (opNumber & 4)
	{
		arr.push("Verify");
	}	
	if (opNumber & 8)
	{
		arr.push("Encrypt");
	}	
	if (opNumber & 16)
	{
		arr.push("Decrypt");
	}
	if (opNumber & 32)
	{
		arr.push("HMAC");
	}

	return arr.join(", ");
}

/*
 * Attach a function to the "Create" Credential buttons.
 * Need to use this because the button will be added dynamically.
 */
$(document).on('click', '.createCredButton', function(){
	var currentButton 	= $(this);

	var currentRow 		= $(this).closest('tr');

	var setName	 		= currentRow.find('td').eq(0).html();
	var version 		= currentRow.find('td').eq(1).html();
	var credType 		= currentRow.find('td').eq(2).html();
	var expiration 		= currentRow.find('input').eq(0).val();

	$.getJSON($SCRIPT_ROOT + '/_create_cred', {
		setName: 		setName,
        version: 		version,
        credType: 			credType,
		expiration: 	expiration,
		primary: 		$("#p_owner").html(),
		secondary: 		$("#s_owner").html()
      }, function(data) {
		// If result is true, hide expiration textbox and replace with text.
		// data.result will be 0 on success.
        if(!data.result)
		{
			// Update row values.
			currentRow.find('td').eq(3).html(expiration);

			// Hide the create button
			currentButton.hide();
		}
		else
		{
			// TODO handle error
			alert('Error creating credential: ' + data.result);
		}
	});

});

/*
 * Attach a function to the 'Edit' buttons.
 * Current only allows operation to be updated.
 * Need to use this because the button will be added dynamically.
 */
$(document).on('click', '.editPermButton', function(){ 
	var currentRow 		= $(this).closest('tr');

	// Replace 'Operation' contents with a multiselect menu.
	var opOptions 		= currentRow.find('td').eq(2).html(operationSelect);

	// Update button function to 'update'.
	$(this).attr('class', 'updatePermButton');
	$(this).text('Save');

});

/*
 * Attach a function to the 'Save' buttons.
 * Need to use this because the button will be added dynamically.
 */
$(document).on('click', '.updatePermButton', function(){
	var currentButton 	= $(this);

	var currentRow 		= $(this).closest('tr');
	var entityRow	 	= currentRow.find('td').eq(0);
	var opOptions 		= currentRow.find('select').eq(0);
	var locRow 			= currentRow.find('td').eq(3);

	// Calculate the operations allowed for this entity (ops)
    var ops = 0;
    opOptions.children('option').each(function(){
		if($(this).is(':selected'))
		{
			ops += parseInt($(this).val());
		}
	});

	$.getJSON($SCRIPT_ROOT + '/_update_perm', {
		setName: 		$("#setName").html(),
        entity: 		entityRow.html(),
		op:				ops,
		loc: 			locRow.html()
      }, function(data) {
		// If result is true, hide input and replace with values.
		// data.result will be 0 on success.
        if(!data.result)
		{
			// Update button function to 'edit'.
			// Show new operation values.
			currentButton.attr('class', 'editPermButton');
			currentButton.text('Edit');
			currentRow.find('td').eq(2).html(getOpString(ops));
		}
		else
		{
			// TODO handle error
			alert('Error updating permission: ' + data.result);
		}
	});
});

/*
 * Attach a function to the "Create" Permission buttons.
 * Need to use this because the button will be added dynamically.
 */
$(document).on('click', '.createPermButton', function(){
	var currentButton 	= $(this);

	var currentRow 		= $(this).closest('tr');
	var entityRow	 	= currentRow.find('input').eq(0);
	var entityOptions 	= currentRow.find('select').eq(0);
	var opOptions 		= currentRow.find('select').eq(1);
	var locRow 			= currentRow.find('input').eq(1);

	// Calculate the operations allowed for this entity (ops)
    var ops = 0;
    opOptions.children('option').each(function(){
		if($(this).is(':selected'))
		{
			ops += parseInt($(this).val());
		}
	});

	$.getJSON($SCRIPT_ROOT + '/_create_perm', {
		setName: 		$("#setName").html(),
        entity: 		entityRow.val(),
        entity_type: 	entityOptions.find(":selected").val(),
		op:				ops,
		loc: 			locRow.val()
      }, function(data) {
		// If result is true, hide input and replace with values.
		// data.result will be 0 on success.
        if(!data.result)
		{
			// Update row values.
			currentRow.find('td').eq(0).html(entityRow.val());
			currentRow.find('td').eq(1).html( getEntityTypeString( parseInt(entityOptions.find(":selected").val()) ) );
			currentRow.find('td').eq(2).html(getOpString(ops));
			currentRow.find('td').eq(3).html(locRow.val());

			// Update button function to 'edit'.
			currentButton.attr('class', 'editPermButton');
			currentButton.text('Edit');

			// Add remove button.
			currentRow.append('<td><button class="removePermButton">Remove</button></td>');
		}
		else
		{
			// TODO handle error
			alert('Error creating permission: ' + data.result);
		}
	});

});

/*
 * Attach a function to the 'Remove' buttons.
 * Need to use this because the button will be added dynamically.
 */
$(document).on('click', '.removePermButton', function(){
	
	var currentButton 	= $(this);

	var currentRow 		= $(this).closest('tr');
	var entityRow	 	= currentRow.find('td').eq(0);
	var locRow 			= currentRow.find('td').eq(3);

	$.getJSON($SCRIPT_ROOT + '/_remove_perm', {
		setName: 		$("#setName").html(),
        entity: 		entityRow.html(),
		loc: 			locRow.html()
      }, function(data) {
		// TODO if result is true, hide row.
		// data.result will be 0 on success.
        if(!data.result)
		{
			currentRow.hide()
		}
		else
		{
			// TODO handle error
			alert('Error removing permission: ' + data.result);
		}
	});
});

/*
 * Occurs when the "Add New Permission" button is clicked.
 * Adds a new row to the permission table.
 */
function addPermRow()
{
	$('#viewPermissions tr:last').after('<tr><td><input type="text"/></td><td>'+entityTypeSelect+'</td><td>'+operationSelect+'</td><td><input type="text"/></td><td><button class="createPermButton">Create</button></td></tr>');
}

/*
 * Occurs when the "Create New Credential" button is clicked.
 * Adds a new row to the credential table.
 */
function addCredRow()
{
	$('#viewSet tr:last').after('<tr><td>'+ $("#setName").html() +'</td><td>'+(1+parseInt($('#viewSet tr:last').find('td').eq(1).html(),10))+'</td><td>'+$('#viewSet tr:last').find('td').eq(2).html()+'</td><td><input type="text"/></td><td><button class="createCredButton">Create</button></td></tr>');
}
