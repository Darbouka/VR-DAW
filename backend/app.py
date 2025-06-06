from flask import Flask, request, jsonify, send_file
from flask_cors import CORS
from flask_jwt_extended import JWTManager, create_access_token, jwt_required, get_jwt_identity
from datetime import timedelta
import os
from dotenv import load_dotenv
from werkzeug.security import generate_password_hash, check_password_hash
from models import db, User, Project, AudioFile, Collaboration
import uuid

# Lade Umgebungsvariablen
load_dotenv()

app = Flask(__name__)
CORS(app)

# Konfiguration
app.config['SQLALCHEMY_DATABASE_URI'] = 'sqlite:///vrdaw.db'
app.config['SQLALCHEMY_TRACK_MODIFICATIONS'] = False
app.config['JWT_SECRET_KEY'] = os.getenv('JWT_SECRET_KEY', 'dev-secret-key')
app.config['JWT_ACCESS_TOKEN_EXPIRES'] = timedelta(hours=1)
app.config['UPLOAD_FOLDER'] = 'uploads'

# Initialisiere Erweiterungen
db.init_app(app)
jwt = JWTManager(app)

# Erstelle Upload-Verzeichnis
os.makedirs(app.config['UPLOAD_FOLDER'], exist_ok=True)

@app.route('/api/auth/register', methods=['POST'])
def register():
    data = request.get_json()
    if User.query.filter_by(username=data['username']).first():
        return jsonify({"error": "Benutzername bereits vergeben"}), 400
    
    user = User(
        username=data['username'],
        password_hash=generate_password_hash(data['password']),
        email=data['email']
    )
    db.session.add(user)
    db.session.commit()
    return jsonify({"message": "Benutzer erfolgreich registriert"}), 201

@app.route('/api/auth/login', methods=['POST'])
def login():
    data = request.get_json()
    user = User.query.filter_by(username=data['username']).first()
    
    if user and check_password_hash(user.password_hash, data['password']):
        access_token = create_access_token(identity=user.id)
        return jsonify(access_token=access_token), 200
    return jsonify({"error": "Ungültige Anmeldedaten"}), 401

@app.route('/api/projects', methods=['GET'])
@jwt_required()
def get_projects():
    user_id = get_jwt_identity()
    projects = Project.query.filter_by(user_id=user_id).all()
    return jsonify([{
        'id': p.id,
        'name': p.name,
        'created_at': p.created_at,
        'updated_at': p.updated_at,
        'files': [{'id': f.id, 'name': f.name} for f in p.files]
    } for p in projects]), 200

@app.route('/api/projects', methods=['POST'])
@jwt_required()
def create_project():
    user_id = get_jwt_identity()
    data = request.get_json()
    
    project = Project(
        name=data['name'],
        user_id=user_id
    )
    db.session.add(project)
    db.session.commit()
    
    return jsonify({
        'id': project.id,
        'name': project.name,
        'created_at': project.created_at
    }), 201

@app.route('/api/projects/<int:project_id>/files', methods=['POST'])
@jwt_required()
def upload_file(project_id):
    user_id = get_jwt_identity()
    project = Project.query.get_or_404(project_id)
    
    if project.user_id != user_id:
        return jsonify({"error": "Keine Berechtigung"}), 403
    
    if 'file' not in request.files:
        return jsonify({"error": "Keine Datei gefunden"}), 400
    
    file = request.files['file']
    if file.filename == '':
        return jsonify({"error": "Keine Datei ausgewählt"}), 400
    
    # Generiere eindeutigen Dateinamen
    filename = str(uuid.uuid4()) + os.path.splitext(file.filename)[1]
    filepath = os.path.join(app.config['UPLOAD_FOLDER'], filename)
    file.save(filepath)
    
    audio_file = AudioFile(
        name=file.filename,
        path=filepath,
        project_id=project_id
    )
    db.session.add(audio_file)
    db.session.commit()
    
    return jsonify({
        'id': audio_file.id,
        'name': audio_file.name,
        'path': audio_file.path
    }), 201

@app.route('/api/collaboration/<int:project_id>', methods=['POST'])
@jwt_required()
def start_collaboration(project_id):
    user_id = get_jwt_identity()
    project = Project.query.get_or_404(project_id)
    
    if project.user_id != user_id:
        return jsonify({"error": "Keine Berechtigung"}), 403
    
    collaboration = Collaboration(
        project_id=project_id,
        user_id=user_id,
        role='owner'
    )
    db.session.add(collaboration)
    db.session.commit()
    
    return jsonify({
        'session_id': f"session_{project_id}_{user_id}",
        'role': collaboration.role
    }), 200

@app.route('/api/collaboration/<int:project_id>/invite', methods=['POST'])
@jwt_required()
def invite_collaborator(project_id):
    user_id = get_jwt_identity()
    project = Project.query.get_or_404(project_id)
    
    if project.user_id != user_id:
        return jsonify({"error": "Keine Berechtigung"}), 403
    
    data = request.get_json()
    collaborator = User.query.filter_by(username=data['username']).first()
    
    if not collaborator:
        return jsonify({"error": "Benutzer nicht gefunden"}), 404
    
    collaboration = Collaboration(
        project_id=project_id,
        user_id=collaborator.id,
        role=data.get('role', 'viewer')
    )
    db.session.add(collaboration)
    db.session.commit()
    
    return jsonify({
        'message': f"Benutzer {collaborator.username} eingeladen",
        'role': collaboration.role
    }), 200

if __name__ == '__main__':
    with app.app_context():
        db.create_all()
    app.run(debug=True, port=8080) 