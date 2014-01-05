from flask import Flask, request, redirect, url_for, session, render_template

app = Flask(__name__)


'''
    The 'home page' for the central authority admin interface.
'''
@app.route('/', methods=['GET', 'POST'])
def index():
    if request.method == 'POST':
        if request.form['submit'] == 'View Set':
            setName = request.form.get('setName')
            return redirect('set/' + setName)
        elif request.form['submit'] == 'Create Set':
            return redirect(url_for('createSet'))
    return render_template('index.html')


'''
    Form for creating a new set.
'''
@app.route('/create/')
def createSet():
    return render_template('createSet.html')


'''
    Page for viewing a set.
'''
@app.route('/set/<setName>/')
def viewSet(setName):
	return render_template('viewSet.html', setName=setName)


"""
'''
    Sample login page.
'''
@app.route('/login/', methods=['GET', 'POST'])
def login():
    if request.method == 'POST':
        username = request.form.get('username')
        password = request.form.get('username')

        # TODO process login
        return redirect(url_for('greet', name=username))

    return render_template('login.html')
"""

if __name__ == '__main__':
    app.run(debug=True)
