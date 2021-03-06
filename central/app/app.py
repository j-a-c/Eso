from flask import Flask, jsonify, request, redirect, session, url_for, render_template
from appExtension import *
import datetime
import os
import re

app = Flask(__name__)

# Secret key used to sign session cookies.
app.secret_key = os.urandom(24)

# Keys for the session map.
LOGGED_IN = 'LOGGED_IN'
USER = 'USER'

'''
    The 'home page' for the central authority admin interface.
'''
@app.route('/', methods=['GET', 'POST'])
def index():

    if request.method == 'POST':
        # View a set.
        if request.form['submit'] == 'View Set':
            setName = request.form.get('setName')
            return redirect('set/' + setName)
        # Create a set enabled if user is logged in.
        elif request.form['submit'] == 'Create Set' and session.get(LOGGED_IN):
            return redirect(url_for('createSet'))
        # Redirect if user tries to create a set while not logged in.
        elif request.form['submit'] == 'Create Set':
            return redirect(url_for('login'))
    return render_template('index.html')


'''
    This method requests that the credential be created by the CA daemon.
    credType is the type of the credential (ex: AES-256). See the credMap for
    accepted credTypes. primary and secondary are the primary and secondary
    owners, respectively.

    This method does some minor checking to make sure the inputs are in the
    correct format, but nothing crazy.

    Returns true if the credential was created successfully.
'''
def requestCreateCredential(setName, version, credType, expiration, primary,
        secondary):
    # Maps the the selected credential type to its algorithm, key size, 
    # and type. Type 1 = credential, 2 = symmetric, 3 = asymmetric.
    # See db_types.h
    # TODO set a safe password size
    credMap = { 'Username / Password'   : ('PASS',  '33', '1'),
                'AES-128'               : ('AES',  '128', '2'),
                'AES-256'               : ('AES',  '256', '2'),
                'RSA-1024'              : ('RSA', '1024', '3'),
                'RSA-2048'              : ('RSA', '2048', '3'),
                'RSA-4096'              : ('RSA', '4096', '3')
            }

    (credAlgo, credSize, credType) = credMap[credType]

    # Are all the contents of the form 'clean'?
    # (In the expected form and satisfy the constraints.)
    isClean = True

    # Check set name.
    if not re.match('^\w+(\.\w+)*$', setName):
        isClean = False

    # Check version.
    # Must be an integer greater than 0 and less than 4294967295 .
    if (not version.isdigit()) or (int(version) <= 0 or int(version) >= 4294967295):
        isClean = False

    # Check expiration.
    # Must be of the form YYYY-MM-DD
    try:
        datetime.datetime.strptime(expiration, '%Y-%m-%d')
    except ValueError:
        isClean = False

    # Check primary.
    # Must be a name with no special characters.
    if not re.match('^\w+$', primary):
        isClean = False

    # Check secondary.
    # Must be a name with no special characters.
    if not re.match('^\w+$', secondary):
        isClean = False

    if not isClean:
        # TODO Error message
        return False

    # Request from the CA daemon that the credential be created.
    if create_credential(setName, version, expiration, primary, secondary,
            credType, credAlgo, credSize) == 0:
        return True

    return False



'''
    Form for creating a new set.
'''
@app.route('/create/', methods=['GET', 'POST'])
def createSet():

    if request.method == 'POST':

        # Get form contents.
        setName = request.form.get('setName')
        version = '1' #request.form.get('version')
        credType = request.form.get('credType')
        expiration = request.form.get('expiration')
        primary = request.form.get('primary')
        secondary = request.form.get('secondary')

        isCreated = requestCreateCredential(setName, version, credType,
                expiration, primary, secondary)

        if not isCreated:
            # TODO Error message and save form input.
            return render_template('createSet.html')

        # If set was created, redirect to its page.
        return redirect('set/' + setName)

    return render_template('createSet.html')


'''
    Page for viewing a set.
'''
@app.route('/set/<setName>/')
def viewSet(setName):

    # Load info from database
    setCreds = get_all_credentials(setName)
    setPerms = get_all_permissions(setName)


    # We will form the string that shows the type of the credential.
    # Because get_all_credentials() returns a tuple, we need to create a copy
    # of the credentials and modify that.
    # cred[4] = primary owner, cred[5] = secondary owner.
    setCredsCopy = []
    for cred in setCreds:
        credTypeString = ''
        if cred[3] == 1:
            credTypeString = 'User/Pass'
        else:
            credTypeString = cred[6] + '-' + str(cred[7])
        setCredsCopy.append( (cred[0], cred[1], credTypeString, cred[3],
            cred[4], cred[5]) )

    # Replaces the old credentials with the modified version.
    setCreds = setCredsCopy

    # We will change the integer representation of entity_type and operation
    # into the string representation for the permission.
    # See the db_types.h for the constants.
    # One problem is that get_all_permissions returns a tuple. Since we cannot
    # modify a tuple, we to create a copy of permissions.
    setPermsCopy = []
    for perm in setPerms:
        # Replace the entity type.
        entityType = ""
        if perm[1] == 1:
            entityType = "User"
        else:
            entityType = "POSIX Group"

        # Create an array of the operation allowed.
        op = perm[2]
        opArray = []
        if op & 1:
            opArray.append("Retrieve")
        if op & 2:
            opArray.append("Sign")
        if op & 4:
            opArray.append("Verify")
        if op & 8:
            opArray.append("Encrypt")
        if op & 16:
            opArray.append("Decrypt")
        if op & 32:
            opArray.append("HMAC")

        # Join the strings and replace the original value.
        opString = ", ".join(opArray)

        # Append the modified tuple.
        setPermsCopy.append( (perm[0], entityType, opString, perm[3]) )

    # Replace the old copy of the permissions with the modified version.
    setPerms = setPermsCopy

    # Set the primary and secondary owners.
    # We are setting the owners here so we can choose whether to generate
    # buttons 'edit' buttons for them later. We want to enforce that only the
    # owners can modify this set.
    primary = None
    secondary = None
    if setCreds:
        primary = setCreds[0][4]
        secondary = setCreds[0][5]

    # We will only allow the user to allow the modify the set if its username
    # matches either the primary or secondary username. We use try/except
    # instead of session.get(USER) because a username might actually be None
    # (or whatever we specify and the default return value from
    # session.get(USER).
    canModify = False
    try:
        if session[USER] == primary or session[USER] == secondary:
            canModify = True
    except KeyError:
        print 'error'
        pass

    return render_template('viewSet.html', setName=setName, setCreds=setCreds,
            setPerms=setPerms, primary=primary, secondary=secondary,
            canModify=canModify)


'''
    Attempt to create a credential.
    Will result json {result:0} if everything went ok and {result:1} otherwise.
'''
@app.route('/_create_cred')
def createCredential():

    # Get message contents.
    setName = request.args.get('setName', '', type=str)
    version = request.args.get('version', '', type=str)
    credType = request.args.get('credType', '', type=str)
    expiration = request.args.get('expiration', '', type=str)
    primary = request.args.get('primary', '', type=str)
    secondary = request.args.get('secondary', '', type=str)

    print setName, version, credType, expiration, primary, secondary

    isCreated = requestCreateCredential(setName, version, credType, expiration,
                primary, secondary)

    # create_permission(...) returns 0 on success, so will we.
    if not isCreated:
        # Something went wrong.
        return jsonify(result=1)
    else:
        # Everything went ok!
        return jsonify(result=0)


'''
    Attempt to create a permission.
    Will result json {result:0} if everything went ok and {result:1} otherwise.
'''
@app.route('/_create_perm')
def createPermission():

    set_name = request.args.get('setName', '', type=str)
    entity = request.args.get('entity', '', type=str)
    entity_type = request.args.get('entity_type', '', type=str)
    op = request.args.get('op', '', type=str)
    loc = request.args.get('loc', '', type=str)

    # Validate input

    # Check entity.
    # Must be a name with no special characters.
    if not re.match('^\w+$', entity):
        return jsonify(result=6)

    # create_permission(...) returns 0 on success, so will we.
    if (create_permission(set_name, entity, entity_type, op, loc)):
        # Something went wrong.
        return jsonify(result=1)
    else:
        # Everything went ok!
        return jsonify(result=0)


'''
    Attempt to update a permission.
    Will result json {result:0} if everything went ok and {result:1} otherwise.
'''
@app.route('/_update_perm')
def updatePermission():
    set_name = request.args.get('setName', '', type=str)
    entity = request.args.get('entity', '', type=str)
    op = request.args.get('op', '', type=str)
    loc = request.args.get('loc', '', type=str)

    # update_permission(...) returns 0 on success, so will we.
    if (update_permission(set_name, entity, op, loc)):
        # Something went wrong.
        return jsonify(result=1)
    else:
        # Everything went ok!
        return jsonify(result=0)

'''
    Attempt to remove a permission.
    Will result json {result:0} if everything went ok and {result:1} otherwise.
'''
@app.route('/_remove_perm')
def removePermission():
    set_name = request.args.get('setName', '', type=str)
    entity = request.args.get('entity', '', type=str)
    loc = request.args.get('loc', '', type=str)

    # remove_permission(...) returns 0 on success, so will we.
    if (remove_permission(set_name, entity, loc)):
        # Something went wrong.
        return jsonify(result=1)
    else:
        # Everything went ok!
        return jsonify(result=0)



'''
    Login page.
'''
@app.route('/login/', methods=['GET', 'POST'])
def login():
    # Handle
    if request.method == 'POST':
        username = request.form.get('username')
        password = request.form.get('password')


        # TODO in a real implementation, this is where the login would be
        # processed. In the debug implementation, the username and password
        # need to be equal.
        if username == password:
            session[LOGGED_IN] = True
            session[USER] = username
            return redirect(url_for('index'))

    return render_template('login.html')

'''
    Logout page.
'''
@app.route('/logout')
def logout():
    session.pop(LOGGED_IN, None)
    return redirect(url_for('index'))

if __name__ == '__main__':
    app.run(debug=True)
